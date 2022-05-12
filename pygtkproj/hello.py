import gi
gi.require_version("Gtk", "3.0")

from gi.repository import Gtk as g
from gi.repository import Gio

from sqlite3 import connect,Connection
from pathlib import Path
from itertools import permutations,product
from sys import argv
from pprint import pprint

# {{{1 db schema
schema =  """ pragma foreign_keys=ON; pragma recursive_triggers=ON;

create table if not exists kvs (
id integer primary key,
k text,
v text,
unique(k) on conflict replace);

create table if not exists paths (
id integer primary key,
name text,
mtime real,
unique (name) on conflict replace);

create table if not exists files (
id integer primary key,
path_id integer,
name text,
mtime real,
unique (name) on conflict replace,
foreign key (path_id) references paths(id) on delete cascade);"""
# }}}1


class Db(Connection):
    def __init__(self,name,**kwargs):
        super().__init__(name,**kwargs)
        self.cu = self.cursor()
        self.cu.row_factory = lambda c,r:r[0]
        made = bool(self.cu.execute("select * from sqlite_master").fetchone())
        if not made:
            self.executescript(schema)
            for a,b in product(
                    ("".join(_) for _ in permutations("hello")),
                    ("".join(_) for _ in permutations("world"))):
                self.execute("insert into kvs(k,v) values(?,?)",(" ".join((a,b)),""))
            self.commit()
        list(map(print,self.iterdump()))
cx = connect("dadbas",factory=Db)
print("cx:",cx)


class Win(g.ApplicationWindow):
    def foo(self,treesel):
        store,iterx = treesel.get_selected()
        oid,k,v = (
            store.get_value(iterx,0),
            store.get_value(iterx,1),
            store.get_value(iterx,2))
        self.sel_idx = oid
        self.inspectframe.set_label(k)
        self.inspectlabel.set_label(v)
        self.inspectentry.set_text(v)
        self.iter_x = iterx
        print("self.iter_x:",self.iter_x)

    def yeetfunc(self,button):
        sel = self.tree_list.get_selection()
        store,iterx = sel.get_selected()
        oid,k,v = (
            store.get_value(iterx,0),
            store.get_value(iterx,1),
            store.get_value(iterx,2))
        cx.execute("delete from kvs where id=?",(oid,))
        cx.commit()
        store.remove(iterx)
        self.iterx = None
        print("removed oid:",oid)
    def entry_func(self,entry):
        if (self.sel_idx > -1) and self.iter_x:
            txt = entry.get_text()
            cx.execute("update kvs set v=? where id=?",(txt,self.sel_idx))
            cx.commit()
            store = self.tree_list.get_model()
            store.set_value(self.iter_x,2,txt)
            print("txt:",txt)

    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.sel_idx = -1
        self.iter_x = None
        store = g.ListStore(int,str,str)
        for r in cx.execute("select * from kvs"):
            store.append(list(r))
        self.box = g.Box(orientation=1,spacing=10)
        self.inspectframe = g.Frame()
        self.inspectframe.set_border_width(10)
        inspectframebox = g.Box(orientation=1,spacing=10)
        self.inspectlabel = g.Label()
        self.inspectentry = g.Entry()
        self.listframe = g.Frame()
        scroll_tree = g.ScrolledWindow()
        self.tree_list = g.TreeView(model=store)
        renderer0 = g.CellRendererText()
        column0 = g.TreeViewColumn("id",renderer0,text=0)
        renderer1 = g.CellRendererText()
        column1 = g.TreeViewColumn("key",renderer1,text=1)
        renderer2 = g.CellRendererText()
        column2 = g.TreeViewColumn("val",renderer2,text=2)
        self.add(self.box)
        self.inspectframe.set_label("foo")
        button = g.Button()
        self.inspectlabel.set_label("bar")
        inspectframebox.add(self.inspectlabel)
        inspectframebox.add(self.inspectentry)
        self.inspectframe.add(inspectframebox)
        self.listframe.set_label("baz")
        scroll_tree.set_vexpand(True)
        self.tree_list.append_column(column0)
        self.tree_list.append_column(column1)
        self.tree_list.append_column(column2)
        self.inspectentry.connect("activate",self.entry_func)
        
        # self.tree_list.connect("row-activated",self.foo)
        self.tree_list.get_selection().connect("changed",self.foo)
        scroll_tree.add(self.tree_list)
        self.listframe.add(scroll_tree)
        button.set_label("yeet")
        button.connect("clicked",self.yeetfunc)
        button.set_margin_start(20)
        button.set_margin_end(20)
        self.box.add(self.inspectframe)
        self.box.add(self.listframe)
        self.box.add(button)
        self.set_border_width(20)
        self.show_all()


class App(g.Application):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.window = None
    def on_quit(self,action,param):
        self.quit()
    def on_info(self,action,param):
        pprint.pprint(dir(self))
    def do_startup(self):
        g.Application.do_startup(self)
        action = Gio.SimpleAction.new("quit", None)
        action.connect("activate", self.on_quit)
        self.add_action(action)
        action = Gio.SimpleAction.new("info", None)
        action.connect("activate", self.on_info)
        self.add_action(action)
        builder = g.Builder.new_from_file("appmenu.xml")
        self.set_app_menu(builder.get_object("app-menu"))
        builder = g.Builder.new_from_file("menu.xml")
        self.set_menubar(builder.get_object("menu"))
    def do_activate(self):
        self.window = Win(application=self,title="Hello World!")
        self.window.present()
app = App(application_id="org.dust.hello_9001")
app.run(argv)

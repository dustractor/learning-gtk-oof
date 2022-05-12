
#include <stdlib.h>
#include <stdio.h>

#include <sqlite3.h>

#include <gtk/gtk.h>

#define LIST_ID 0
#define LIST_K 1
#define LIST_V 2
#define N_COLS 3

char * sql_ddl =
    "create table if not exists data ("
    "id integer primary key,"
    "name text,"
    "value text,"
    "unique(name) on conflict replace);";

char *sql_dummy = "insert into data(name,value) values('foo','bar');";
char *sql_insert_data = "insert into data(name,value) values(:k,:v);";
char *sql_select = "select * from data";

struct ProgramData {
    sqlite3 *db;
    GtkWidget *labelref;
    GtkEntry *entry_k;
    GtkEntry *entry_v;
};

struct ProgramData pdata;

GtkCellRenderer *renderer;
GtkListStore *store;
GtkTreeSelection *selection;
GtkTreeViewColumn *tv_column;
GtkWidget *menu_bar;
GtkWidget *menu_item_msg1;
GtkWidget *menu_item_quit;
GtkWidget *menu_item_x0;
GtkWidget *submenu_alpha;
GtkWidget *tree_list;
GtkWidget *frame_t;
GtkWidget *box_t;
GtkWidget *frame_k;
GtkWidget *frame_v;
GtkWidget *frame_m;
GtkWidget *box_m;

int factory(void *, int, char **, char **);

static void tree_selection_changed_callback(GtkTreeSelection *selection,gpointer data);

static void quit_action(GtkMenuItem *menuitem,gpointer data) {
    exit(0);
}

static void on_entry_activated(GtkEntry *entry_k, gpointer data) {
    g_print(gtk_entry_buffer_get_text(gtk_entry_get_buffer(entry_k)));
}

static void on_entry2_activated(GtkEntry *entry_k, gpointer data) {
    g_print(gtk_entry_buffer_get_text(gtk_entry_get_buffer(pdata.entry_k)));
    g_print(gtk_entry_buffer_get_text(gtk_entry_get_buffer(pdata.entry_v)));
}

static void on_button_clicked(GtkButton *button, gpointer data) {
    const char * key;
    const char * val;
    char *err_msg = 0;
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_open("test.db",&pdata.db);
    int idx = -1;
    if (rc != SQLITE_OK){
        fprintf(stderr,"db error: %s\n",sqlite3_errmsg(pdata.db));
        sqlite3_close(pdata.db);
        return;
    }
    key = gtk_entry_buffer_get_text(gtk_entry_get_buffer(pdata.entry_k));
    val = gtk_entry_buffer_get_text(gtk_entry_get_buffer(pdata.entry_v));
    g_print("key:%s\n",key);
    g_print("val:%s\n",val);
    rc = sqlite3_prepare_v2(pdata.db,sql_insert_data,-1,&stmt,NULL);
    idx = sqlite3_bind_parameter_index(stmt,":k");
    rc = sqlite3_bind_text(stmt,idx,key,-1,SQLITE_STATIC);
    idx = sqlite3_bind_parameter_index(stmt,":v");
    rc = sqlite3_bind_text(stmt,idx,val,-1,SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    rc = sqlite3_exec(pdata.db,sql_select,factory,store,&err_msg);
    gtk_button_set_label(button,"Hello World");
    gtk_label_set_markup((GtkLabel *) pdata.labelref,"<span size='73728'>foo bar baz</span>");
}

int factory(void *model,int argc,char **argv,char ** colname){
    GtkTreeIter iter;
    for (int i=0;i<argc;i++){
        printf("%s = %s\n",colname[i],argv[i] ? argv[i]:"NULL");
    }
    gtk_list_store_append(GTK_LIST_STORE(model),&iter);
    gtk_list_store_set(
            GTK_LIST_STORE(model),
            &iter,
            LIST_ID,argv[0],
            LIST_K,argv[1],
            LIST_V,argv[2],
            -1);
    return 0;
}

static void tree_selection_changed_callback(GtkTreeSelection *selection, gpointer data){
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar * rowid;
    gchar * k;
    gchar * v;
    if (gtk_tree_selection_get_selected(selection,&model,&iter)){
        gtk_tree_model_get(
                model,&iter,
                LIST_ID,&rowid,
                LIST_K,&k,
                LIST_V,&v,
                -1);
        gtk_entry_set_text(pdata.entry_k,k);
        gtk_entry_set_text(pdata.entry_v,v);
        g_print("rowid: %s\n", rowid);
        g_print("k: %s\n", k);
        g_print("v: %s\n", v);
        g_free(rowid);
        g_free(k);
        g_free(v);
    }
}

static void on_app_activate(GApplication *app, gpointer data) {
    store = gtk_list_store_new(N_COLS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    char *err_msg = 0;
    int rc = sqlite3_open("test.db",&pdata.db);
    if (rc != SQLITE_OK){ fprintf(stderr,"db error: %s\n",sqlite3_errmsg(pdata.db)); sqlite3_close(pdata.db); return; }
    rc = sqlite3_exec(pdata.db,sql_ddl,0,0,&err_msg);
    if (rc != SQLITE_OK){ fprintf(stderr,"sql error: %s\n",err_msg); sqlite3_close(pdata.db); return; }
    sqlite3_free(err_msg);
    rc = sqlite3_exec(pdata.db,sql_select,factory,store,&err_msg);
    if (rc != SQLITE_OK){ fprintf(stderr,"sql error: %s\n",err_msg); sqlite3_close(pdata.db); return; }
    sqlite3_free(err_msg);
    sqlite3_close(pdata.db);
    GtkWidget *appwindow = gtk_application_window_new(GTK_APPLICATION(app));
    GtkWidget *mainbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    GtkWidget *button = gtk_button_new_with_label("Click Me!");
    pdata.entry_k = (GtkEntry*) gtk_entry_new();
    pdata.entry_v = (GtkEntry*) gtk_entry_new();
    GtkWidget *frame_k = gtk_frame_new("Key:");
    GtkWidget *frame_v = gtk_frame_new("Value:");
    GtkWidget *label = gtk_label_new("label");
    GtkWidget *label2 = gtk_label_new("other label");
    GtkWidget *frame_m = gtk_frame_new("Operations");
    GtkWidget *frame_t = gtk_frame_new("Data:");
    GtkWidget *box_m = gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    GtkWidget *scrollview = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_min_content_height(scrollview,400);
    menu_bar = gtk_menu_bar_new();
    menu_item_x0 = gtk_menu_item_new_with_mnemonic("_File");
    submenu_alpha = gtk_menu_new();
    menu_item_msg1 = gtk_menu_item_new_with_label("Hello World");
    menu_item_quit = gtk_menu_item_new_with_mnemonic("_Quit");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu_alpha),menu_item_msg1);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu_alpha),menu_item_quit);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item_x0),submenu_alpha);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar),menu_item_x0);
    g_signal_connect(menu_item_quit,"activate",G_CALLBACK(quit_action), NULL);
    tree_list = gtk_tree_view_new();
    renderer = gtk_cell_renderer_text_new();
    tv_column = gtk_tree_view_column_new_with_attributes("ID",renderer,"text",LIST_ID,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_list),tv_column);
    renderer = gtk_cell_renderer_text_new();
    tv_column = gtk_tree_view_column_new_with_attributes("NAME",renderer,"text",LIST_K,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_list),tv_column);
    renderer = gtk_cell_renderer_text_new();
    tv_column = gtk_tree_view_column_new_with_attributes("VALUE",renderer,"text",LIST_V,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_list),tv_column);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_list),GTK_TREE_MODEL(store));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_list));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection),"changed",G_CALLBACK(tree_selection_changed_callback),NULL);
    g_object_unref(store);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_list),FALSE);
    box_t = gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    gtk_container_add(GTK_CONTAINER(scrollview),tree_list);
    gtk_box_pack_start(GTK_BOX(box_t),scrollview,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(box_t),label2,FALSE,FALSE,5);
    pdata.labelref = label;
    g_signal_connect(button,"clicked",G_CALLBACK(on_button_clicked),NULL);
    g_signal_connect(pdata.entry_k,"activate",G_CALLBACK(on_entry_activated),NULL);
    g_signal_connect(pdata.entry_v,"activate",G_CALLBACK(on_entry2_activated),NULL);
    gtk_container_add(GTK_CONTAINER(appwindow),mainbox);
    gtk_container_add(GTK_CONTAINER(mainbox),menu_bar);
    gtk_container_add(GTK_CONTAINER(frame_m),box_m);
    gtk_container_add(GTK_CONTAINER(box_m),button);
    gtk_container_add(GTK_CONTAINER(box_m),label);
    gtk_container_add(GTK_CONTAINER(mainbox),frame_m);
    gtk_container_add(GTK_CONTAINER(frame_k), (GtkWidget*) pdata.entry_k);
    gtk_container_add(GTK_CONTAINER(frame_v), (GtkWidget*) pdata.entry_v);
    gtk_container_add(GTK_CONTAINER(mainbox),frame_k);
    gtk_container_add(GTK_CONTAINER(mainbox),frame_v);
    gtk_container_add(GTK_CONTAINER(mainbox),box_t);
    gtk_widget_show_all(GTK_WIDGET(appwindow));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
            "org.gtkmm.example.HelloApp",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(on_app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return status;
}


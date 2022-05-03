#include <sqlite3.h>
#include <gtk/gtk.h>
int callback(void *, int, char **, char **);

enum {
    LIST_ID,
    LIST_K,
    LIST_V,
    N_COLS
};

GtkWidget * labelref;

static void tree_selection_changed_callback(GtkTreeSelection *selection,gpointer data);

static void on_button_clicked(GtkButton *button, gpointer data) {
    gtk_button_set_label(button, "Hello World");
    gtk_label_set_markup((GtkLabel *) labelref,"<span size='48000'>foo bar baz</span>");
}

static void on_app_activate(GApplication *app, gpointer data) {
    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(app));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL,10);
    GtkWidget *button = gtk_button_new_with_label("Click Me!");
    GtkWidget *label = gtk_label_new("label");
    GtkWidget *label2 = gtk_label_new("other label");
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;
    GtkWidget *list;
    GtkWidget *vbox;
    GtkTreeSelection *selection;
    store = gtk_list_store_new(N_COLS,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("test.db",&db);
    if (rc != SQLITE_OK){
        fprintf(stderr,"db error: %s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char *sql_ddl = "create table if not exists data (name,value);";
    char *sql_dummy = "insert into data(name,value) values('foo','bar');";
    char *sql_select = "select * from data";
    rc = sqlite3_exec(db,sql_ddl,0,0,&err_msg);
    if (rc != SQLITE_OK){
        fprintf(stderr,"sql error: %s\n",err_msg);
        sqlite3_close(db);
        return;
    }
    rc = sqlite3_exec(db,sql_dummy,0,0,&err_msg);
    if (rc != SQLITE_OK){
        fprintf(stderr,"sql error: %s\n",err_msg);
        sqlite3_close(db);
        return;
    }
    rc = sqlite3_exec(db,sql_select,callback,store,&err_msg);
    if (rc != SQLITE_OK){
        fprintf(stderr,"sql error: %s\n",err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }
    sqlite3_close(db);
    list = gtk_tree_view_new();
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
            "ID",renderer,"text",LIST_ID,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list),column);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
            "NAME",renderer,"text",LIST_K,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list),column);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes(
            "VALUE",renderer,"text",LIST_V,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list),column);
    gtk_tree_view_set_model(GTK_TREE_VIEW(list),GTK_TREE_MODEL(store));
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
    gtk_tree_selection_set_mode(selection,GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection),"changed",
            G_CALLBACK(tree_selection_changed_callback),
            NULL);
    g_object_unref(store);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list),FALSE);
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_box_pack_start(GTK_BOX(vbox),list,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(vbox),label2,FALSE,FALSE,5);
    labelref = label;
    g_signal_connect(button, "clicked", G_CALLBACK(on_button_clicked), NULL);
    gtk_container_add(GTK_CONTAINER(window),box);
    gtk_container_add(GTK_CONTAINER(box),button);
    gtk_container_add(GTK_CONTAINER(box),label);
    gtk_container_add(GTK_CONTAINER(box),vbox);
    gtk_widget_show_all(GTK_WIDGET(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new(
            "org.gtkmm.example.HelloApp",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(on_app_activate), NULL);
    int status = g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return status;
}

int callback(void *model,int argc,char **argv,char ** azColName){
    GtkTreeIter iter;
    for (int i=0;i<argc;i++){
        printf("%s = %s\n",azColName[i],argv[i] ? argv[i]:"NULL");
    }
    gtk_list_store_append(GTK_LIST_STORE(model),&iter);
    gtk_list_store_set(GTK_LIST_STORE(model),&iter,LIST_ID,argv[0],
            LIST_K,argv[1],
            LIST_V,argv[2],
            -1);
    return 0;
}

static void tree_selection_changed_callback(
        GtkTreeSelection *selection,gpointer data){
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar * k;
    if (gtk_tree_selection_get_selected(selection,&model,&iter)){
        gtk_tree_model_get(model,&iter,LIST_ID,&k,-1);
        g_print("k: %s", k);
        g_free(k);
    }
}

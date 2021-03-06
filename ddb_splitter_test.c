#include <gtk/gtk.h>
#include "ddb_splitter.h"

static void
add_clicked(GtkToolButton *button, GtkWidget *splitter)
{
    GtkWidget *widget;
    widget = gtk_button_new_with_label("New button");

    /* Show the widget and add it to our container */
    if (widget) {
        gtk_widget_show(widget);
        gtk_container_add(GTK_CONTAINER (splitter), widget);
    }
}

static void
lock1_clicked(GtkToolButton *button, GtkWidget *splitter)
{
    DdbSplitterSizeMode mode = ddb_splitter_get_size_mode (DDB_SPLITTER (splitter));
    if (mode != DDB_SPLITTER_SIZE_MODE_LOCK_C1) {
        ddb_splitter_set_size_mode (DDB_SPLITTER (splitter), DDB_SPLITTER_SIZE_MODE_LOCK_C1);
    }
}

static void
lock2_clicked(GtkToolButton *button, GtkWidget *splitter)
{
    DdbSplitterSizeMode mode = ddb_splitter_get_size_mode (DDB_SPLITTER (splitter));
    if (mode != DDB_SPLITTER_SIZE_MODE_LOCK_C2) {
        ddb_splitter_set_size_mode (DDB_SPLITTER (splitter), DDB_SPLITTER_SIZE_MODE_LOCK_C2);
    }
}

static void
prop_clicked(GtkToolButton *button, GtkWidget *splitter)
{
    DdbSplitterSizeMode mode = ddb_splitter_get_size_mode (DDB_SPLITTER (splitter));
    if (mode != DDB_SPLITTER_SIZE_MODE_PROP) {
        ddb_splitter_set_size_mode (DDB_SPLITTER (splitter), DDB_SPLITTER_SIZE_MODE_PROP);
    }
}

static void
remove_clicked(GtkToolButton *button, GtkWidget *splitter)
{
    ddb_splitter_remove_c1 (DDB_SPLITTER (splitter));
    ddb_splitter_remove_c2 (DDB_SPLITTER (splitter));
}

void
scale_button_value_changed (GtkScaleButton *button,
        gdouble         value,
        gpointer        user_data)
{
    GtkWidget *splitter = GTK_WIDGET (user_data);
    ddb_splitter_set_proportion (DDB_SPLITTER (splitter), gtk_scale_button_get_value (button));
}

int
main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    /* Build the main window */
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "DdbSplitter Test");
    //gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (window, 800, 400);

    GtkSettings *default_settings = gtk_settings_get_default();
    g_object_set(default_settings, "gtk-button-images", FALSE, NULL);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    /* Build the various other widgets */
    GtkWidget *vbox = gtk_vbox_new (FALSE, 10);
    GtkWidget *toolbar = gtk_toolbar_new();
    GtkToolItem *add = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
    GtkToolItem *remove = gtk_tool_button_new_from_stock (GTK_STOCK_REMOVE);
    GtkToolItem *lock1 = gtk_tool_button_new (NULL, "Lock child1");
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (lock1), "Lock child 1");
    GtkToolItem *lock2 = gtk_tool_button_new (NULL, "Lock child2");
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (lock2), "Lock child 2");
    GtkToolItem *prop = gtk_tool_button_new (NULL, "Prop sizing");
    gtk_tool_button_set_label (GTK_TOOL_BUTTON (prop), "Prop sizing");
    GtkToolItem *tool_box = gtk_tool_item_new ();
    G_GNUC_END_IGNORE_DEPRECATIONS

    GtkWidget *splitter = ddb_splitter_new (GTK_ORIENTATION_HORIZONTAL);

    GtkWidget *scale = gtk_scale_button_new (GTK_ICON_SIZE_SMALL_TOOLBAR, 0.0, 1.0, 0.01, NULL);
    gtk_scale_button_set_value (GTK_SCALE_BUTTON (scale), ddb_splitter_get_proportion (DDB_SPLITTER (splitter)));
    gtk_container_add (GTK_CONTAINER (tool_box), scale);

    /* Connect signals */
    g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect (add, "clicked", G_CALLBACK(add_clicked), splitter);
    g_signal_connect (remove, "clicked", G_CALLBACK(remove_clicked), splitter);
    g_signal_connect (lock1, "clicked", G_CALLBACK(lock1_clicked), splitter);
    g_signal_connect (lock2, "clicked", G_CALLBACK(lock2_clicked), splitter);
    g_signal_connect (prop, "clicked", G_CALLBACK(prop_clicked), splitter);
    g_signal_connect (scale, "value-changed", G_CALLBACK(scale_button_value_changed), splitter);

    /* Put all the widgets together */
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (add), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (remove), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (lock1), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (lock2), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (prop), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (tool_box), -1);
    gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET (toolbar), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), splitter, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER(window), vbox);
    gtk_widget_show_all (window);

    /* Run the program */
    gtk_main();
    return 0;
}

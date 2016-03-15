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
remove_clicked(GtkToolButton *button, GtkWidget *square)
{
    //gtk_container_remove(GTK_CONTAINER(splitter), last->data);
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
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (window, 400, 400);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    /* Build the various other widgets */
    GtkWidget *vbox = gtk_vbox_new (FALSE, 10);
    GtkWidget *toolbar = gtk_toolbar_new();
    GtkToolItem *add = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
    GtkToolItem *remove = gtk_tool_button_new_from_stock (GTK_STOCK_REMOVE);
    GtkToolItem *tool_box = gtk_tool_item_new ();
    G_GNUC_END_IGNORE_DEPRECATIONS

    GtkWidget *splitter = ddb_splitter_new (GTK_ORIENTATION_VERTICAL);

    GtkWidget *scale = gtk_scale_button_new (GTK_ICON_SIZE_SMALL_TOOLBAR, 0.0, 1.0, 0.01, NULL);
    gtk_scale_button_set_value (GTK_SCALE_BUTTON (scale), ddb_splitter_get_proportion (DDB_SPLITTER (splitter)));
    gtk_container_add (GTK_CONTAINER (tool_box), scale);

    /* Connect signals */
    g_signal_connect (window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect (add, "clicked", G_CALLBACK(add_clicked), splitter);
    g_signal_connect (remove, "clicked", G_CALLBACK(remove_clicked), splitter);
    g_signal_connect (scale, "value-changed", G_CALLBACK(scale_button_value_changed), splitter);

    /* Put all the widgets together */
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (add), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (remove), -1);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (tool_box), -1);
    gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET (toolbar), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), splitter, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER(window), vbox);
    gtk_widget_show_all (window);

    /* Run the program */
    gtk_main();
    return 0;
}

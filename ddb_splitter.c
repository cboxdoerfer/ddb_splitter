/*
 * Copyright (c) 2016 Christian Boxdörfer <christian.boxdoerfer@posteo.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <math.h>
#include "ddb_splitter.h"

/**
 * SECTION: ddb-splitter
 * @title: DdbSplitter
 * @short_description: A container widget similar to GtkPaned, but with
 * the ability to use proportional resizing
 **/

#define DDB_SPLITTER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
            DDB_TYPE_SPLITTER, DdbSplitterPrivate))

/* Property identifiers */
enum
{
    PROP_0,
    PROP_ORIENTATION,
    PROP_SIZE_MODE,
    PROP_PROPORTION,
};

static void
ddb_splitter_get_property (GObject *object,
                           guint prop_id,
                           GValue *value,
                           GParamSpec *pspec);
static void
ddb_splitter_set_property (GObject *object,
                           guint prop_id,
                           const GValue *value,
                           GParamSpec *pspec);

static void
ddb_splitter_set_orientation (DdbSplitter *splitter, GtkOrientation orientation);

//static void
//ddb_splitter_size_request (GtkWidget *widget,
//                           GtkRequisition *requisition);
static void
ddb_splitter_size_allocate (GtkWidget *widget,
                            GtkAllocation *allocation);
static void
ddb_splitter_add (GtkContainer *container,
                  GtkWidget *widget);
static void
ddb_splitter_remove (GtkContainer *container,
                     GtkWidget *widget);
static void
ddb_splitter_forall (GtkContainer *container,
                     gboolean include_internals,
                     GtkCallback callback,
                     gpointer callback_data);

struct _DdbSplitterPrivate
{
    GtkWidget *child1;
    GtkWidget *child2;
    guint child1_size;
    guint child2_size;

    GdkWindow *handle;
    GdkRectangle handle_pos;

    /* configurable parameters */
    GtkOrientation orientation;
    DdbSplitterSizeMode size_mode;
    gfloat proportion;
};

G_DEFINE_TYPE (DdbSplitter, ddb_splitter, GTK_TYPE_CONTAINER)

static void
ddb_splitter_class_init (DdbSplitterClass *klass)
{
    GtkContainerClass *gtkcontainer_class;
    GtkWidgetClass    *gtkwidget_class;
    GObjectClass      *gobject_class;

    /* add our private data to the class */
    g_type_class_add_private (klass, sizeof (DdbSplitterPrivate));

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->get_property = ddb_splitter_get_property;
    gobject_class->set_property = ddb_splitter_set_property;

    gtkwidget_class = GTK_WIDGET_CLASS (klass);
    //gtkwidget_class->size_request = ddb_splitter_size_request;
    gtkwidget_class->size_allocate = ddb_splitter_size_allocate;

    gtkcontainer_class = GTK_CONTAINER_CLASS (klass);
    gtkcontainer_class->add = ddb_splitter_add;
    gtkcontainer_class->remove = ddb_splitter_remove;
    gtkcontainer_class->forall = ddb_splitter_forall;

    /**
     * DdbSplitter::size_mode:
     *
     * The size mode of the splitter.
     **/
    g_object_class_install_property (gobject_class,
            PROP_SIZE_MODE,
            g_param_spec_enum ("size-mode",
                "Size mode",
                "The size mode of the splitter widget",
                DDB_SPLITTER_TYPE_SIZE_MODE, DDB_SPLITTER_SIZE_MODE_PROP,
                G_PARAM_READWRITE));
    /**
     * DdbSplitter::orientation:
     *
     * The orientation of the splitter.
     **/
    g_object_class_install_property (gobject_class,
            PROP_ORIENTATION,
            g_param_spec_enum ("orientation",
                "Orientation",
                "The orientation of the splitter widget",
                GTK_TYPE_ORIENTATION, GTK_ORIENTATION_HORIZONTAL,
                G_PARAM_READWRITE));
    /**
     * DdbSplitter::proportion:
     *
     * The percentage of space allocated to the first child.
     **/
    g_object_class_install_property (gobject_class,
            PROP_PROPORTION,
            g_param_spec_float ("proportion",
                "Proportion",
                "The percentage of space allocated to the first child",
                -G_MAXFLOAT, 1.0, -1.0,
                G_PARAM_READWRITE));
}

static void
ddb_splitter_init (DdbSplitter *splitter)
{
    /* grab a pointer on the private data */
    splitter->priv = DDB_SPLITTER_GET_PRIVATE (splitter);

    splitter->priv->orientation = GTK_ORIENTATION_HORIZONTAL;
    splitter->priv->size_mode = DDB_SPLITTER_SIZE_MODE_PROP;
    splitter->priv->child1 = NULL;
    splitter->priv->child2 = NULL;
    splitter->priv->child1_size = 0;
    splitter->priv->child2_size = 0;
    splitter->priv->handle_pos.x = -1;
    splitter->priv->handle_pos.y = -1;
    splitter->priv->handle_pos.width = 5;
    splitter->priv->handle_pos.height = 5;
    splitter->priv->proportion = 0.5f;
    /* we don't provide our own window */
    gtk_widget_set_has_window (GTK_WIDGET (splitter), FALSE);
}

static void
ddb_splitter_get_property (GObject *object,
                           guint prop_id,
                           GValue *value,
                           GParamSpec *pspec)
{
    DdbSplitter *splitter = DDB_SPLITTER (object);

    switch (prop_id)
    {
        case PROP_ORIENTATION:
            g_value_set_enum (value, ddb_splitter_get_orientation (splitter));
            break;

        case PROP_SIZE_MODE:
            g_value_set_enum (value, ddb_splitter_get_size_mode (splitter));
            break;

        case PROP_PROPORTION:
            g_value_set_float (value, ddb_splitter_get_proportion (splitter));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void
ddb_splitter_set_property (GObject *object,
                           guint prop_id,
                           const GValue *value,
                           GParamSpec *pspec)
{
    DdbSplitter *splitter = DDB_SPLITTER (object);

    switch (prop_id)
    {
        case PROP_ORIENTATION:
            ddb_splitter_set_orientation (splitter, g_value_get_enum (value));
            break;

        case PROP_SIZE_MODE:
            ddb_splitter_set_size_mode (splitter, g_value_get_enum (value));
            break;

        case PROP_PROPORTION:
            ddb_splitter_set_proportion (splitter, g_value_get_float (value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

//static void
//ddb_splitter_size_request (GtkWidget      *widget,
//        GtkRequisition *requisition)
//{
//    DdbSplitter *splitter = DDB_SPLITTER (widget);
//
//    gint border_width = 0;
//    GtkRequisition req_c1;
//    req_c1.width = 0;
//    req_c1.height = 0;
//    if (gtk_widget_get_visible (splitter->priv->child1)) {
//        gtk_widget_size_request (splitter->priv->child1, &req_c1);
//    }
//
//    GtkRequisition req_c2;
//    req_c2.width = 0;
//    req_c2.height = 0;
//    if (gtk_widget_get_visible (splitter->priv->child2)) {
//        gtk_widget_size_request (splitter->priv->child2, &req_c2);
//    }
//
//    requisition->width += border_width * 2;
//    requisition->height += border_width * 2;
//
//    requisition->width += MAX (req_c1.width, req_c2.width);
//    requisition->height += MAX (req_c1.height, req_c2.height);
//
//    // TODO: handle size
//}
//
static void
ddb_splitter_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
    DdbSplitter *splitter = DDB_SPLITTER (widget);
    GtkWidget *c1 = splitter->priv->child1;
    GtkWidget *c2 = splitter->priv->child2;
    // TODO: consider border width
    gint border_width = 0;
    gtk_widget_set_allocation (widget, allocation);

    gboolean child1_visible = c1 && gtk_widget_get_visible (c1) ? TRUE : FALSE;
    gboolean child2_visible = c2 && gtk_widget_get_visible (c2) ? TRUE : FALSE;
    guint num_visible_children = child1_visible + child2_visible;

    gint con_width = allocation->width - border_width * 2;
    gint con_height = allocation->height - border_width * 2;
    gint handle_size = 0;

    GtkAllocation child1_allocation;
    GtkAllocation child2_allocation;
    if (splitter->priv->orientation == GTK_ORIENTATION_HORIZONTAL) {
        handle_size = num_visible_children > 1 ? splitter->priv->handle_pos.width : 0;
        if (child1_visible) {
            // use full height in horitzontal splitter
            child1_allocation.height = MAX (1, con_height);

            gint width = 0;
            if (splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C1) {
                // child 1 locked, use saved size
                width = splitter->priv->child1_size;
            }
            else {
                if (num_visible_children == 1) {
                    // only one child, use full width
                    width = con_width;
                }
                else if (child2_visible && splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C2) {
                    // two children and second one is locked, use all space left
                    width = con_width - splitter->priv->child2_size - handle_size;
                }
                else {
                    // two children visible and proportional scaling is active
                    width = (con_width - handle_size) * splitter->priv->proportion;
                }
            }
            child1_allocation.width = MAX (1, width);
            child1_allocation.x =allocation->x + border_width;
            child1_allocation.y = allocation->y + border_width;

            gtk_widget_size_allocate (splitter->priv->child1, &child1_allocation);
            splitter->priv->child1_size = child1_allocation.width;
            if (splitter->priv->size_mode != DDB_SPLITTER_SIZE_MODE_PROP) {
                splitter->priv->proportion = (float)child1_allocation.width/(con_width - handle_size);
            }
        }
        if (child2_visible) {
            // use full height in horitzontal splitter
            child2_allocation.height = MAX (1, con_height);
            gint width = 0;
            if (splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C2) {
                // child 2 locked, use saved size
                width = splitter->priv->child2_size;
            }
            else {
                if (num_visible_children == 1) {
                    // only one child, use full width
                    width = con_width;
                }
                else if (child1_visible && splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C1) {
                    // two children and first one is locked, use all space left
                    width = con_width - splitter->priv->child1_size - handle_size;
                }
                else {
                    // two children visible and proportional scaling is active
                    width = con_width - child1_allocation.width - handle_size;
                }
            }
            child2_allocation.width = MAX (1, width);
            child2_allocation.x = child1_allocation.x + child1_allocation.width + handle_size;
            child2_allocation.y = allocation->y + border_width;

            gtk_widget_size_allocate (splitter->priv->child2, &child2_allocation);
            splitter->priv->child2_size = child2_allocation.width;
        }
    }
    else {
        // splitter->priv->orientation == GTK_ORIENTATION_VERTICAL

        handle_size = num_visible_children > 1 ? splitter->priv->handle_pos.height : 0;
        if (child1_visible) {
            // use full width in vertical splitter
            child1_allocation.width = MAX (1, con_width);

            gint height = 0;
            if (splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C1) {
                // child 1 locked, use saved size
                height = splitter->priv->child1_size;
            }
            else {
                if (num_visible_children == 1) {
                    // only one child, use full height
                    height = con_height;
                }
                else if (child2_visible && splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C2) {
                    // two children and second one is locked, use all space left
                    height = con_height - splitter->priv->child2_size - handle_size;

                }
                else {
                    // two children visible and proportional scaling is active
                    height = (con_height - handle_size) * splitter->priv->proportion;
                }
            }
            child1_allocation.height = MAX (1, height);
            child1_allocation.x =allocation->x + border_width;
            child1_allocation.y = allocation->y + border_width;

            gtk_widget_size_allocate (splitter->priv->child1, &child1_allocation);
            splitter->priv->child1_size = child1_allocation.height;
            if (splitter->priv->size_mode != DDB_SPLITTER_SIZE_MODE_PROP) {
                splitter->priv->proportion = (float)child1_allocation.height/(con_height - handle_size);
            }
        }
        if (child2_visible) {
            // use full width in vertical splitter
            child2_allocation.width = MAX (1, con_width);

            gint height = 0;
            if (splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C2) {
                // child 2 locked, use saved size
                height = splitter->priv->child2_size;
            }
            else {
                if (num_visible_children == 1) {
                    // only one child, use full height
                    height = con_height;
                }
                else if (child1_visible && splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_LOCK_C1) {
                    // two children and first one is locked, use all space left
                    height = con_height - splitter->priv->child1_size - handle_size;
                }
                else {
                    // two children visible and proportional scaling is active
                    height = con_height - child1_allocation.height - handle_size;
                }
            }
            child2_allocation.height = MAX (1, height);
            child2_allocation.x = allocation->x + border_width;
            child2_allocation.y = child1_allocation.y + child1_allocation.height + handle_size;

            gtk_widget_size_allocate (splitter->priv->child2, &child2_allocation);
            splitter->priv->child2_size = child2_allocation.height;
        }
    }

    if (!child1_visible && !child2_visible) {
        GtkAllocation child_allocation;

        if (splitter->priv->child1)
            gtk_widget_set_child_visible (splitter->priv->child1, TRUE);
        if (splitter->priv->child2)
            gtk_widget_set_child_visible (splitter->priv->child2, TRUE);

        child_allocation.x = allocation->x + border_width;
        child_allocation.y = allocation->y + border_width;
        child_allocation.width = MAX (1, con_width);
        child_allocation.height = MAX (1, con_height);

        if (splitter->priv->child1 && gtk_widget_get_visible (splitter->priv->child1))
            gtk_widget_size_allocate (splitter->priv->child1, &child_allocation);
        else if (splitter->priv->child2 && gtk_widget_get_visible (splitter->priv->child2))
            gtk_widget_size_allocate (splitter->priv->child2, &child_allocation);
    }
}

static void
ddb_splitter_add (GtkContainer *container, GtkWidget *widget)
{
    DdbSplitter *splitter = DDB_SPLITTER (container);

    if (splitter->priv->child1 && splitter->priv->child2) {
        // Splitter already full
        return;
    }

    gtk_widget_set_parent (widget, GTK_WIDGET (splitter));

    if (!splitter->priv->child1) {
        splitter->priv->child1 = widget;
    }
    else if (!splitter->priv->child2) {
        splitter->priv->child2 = widget;
    }

    /* realize the widget if required */
    if (gtk_widget_get_realized (GTK_WIDGET (container)))
        gtk_widget_realize (widget);

    /* map the widget if required */
    if (gtk_widget_get_visible (GTK_WIDGET (container)) && gtk_widget_get_visible (widget))
    {
        if (gtk_widget_get_mapped (GTK_WIDGET (container)))
            gtk_widget_map (widget);
    }

    gtk_widget_queue_resize (GTK_WIDGET (container));
    return;
}

static void
ddb_splitter_remove (GtkContainer *container, GtkWidget *widget)
{
    DdbSplitter *splitter = DDB_SPLITTER (container);

    /* check if the widget was visible */
    gboolean widget_was_visible = gtk_widget_get_visible (widget);

    /* unparent and remove the widget */
    gtk_widget_unparent (widget);
    if (splitter->priv->child1 == widget) {
        splitter->priv->child1 = NULL;
    }
    else if (splitter->priv->child2 == widget) {
        splitter->priv->child2 = NULL;
    }

    if (G_LIKELY (widget_was_visible))
        gtk_widget_queue_resize (GTK_WIDGET (splitter));
}

static void
ddb_splitter_forall (GtkContainer *container,
                     gboolean include_internals,
                     GtkCallback callback,
                     gpointer callback_data)
{
    DdbSplitter *splitter = DDB_SPLITTER (container);
    GtkWidget *c1 = splitter->priv->child1;
    GtkWidget *c2 = splitter->priv->child2;

    if (c1 && GTK_IS_WIDGET (c1)) {
        (*callback) (c1, callback_data);
    }
    if (c2 && GTK_IS_WIDGET (c2)) {
        (*callback) (c2, callback_data);
    }
}

static void
ddb_splitter_remove_child (DdbSplitter *splitter, guint child)
{
    if (child == 1 && splitter->priv->child1) {
        ddb_splitter_remove (GTK_CONTAINER (splitter), splitter->priv->child1);
    }
    else if (child == 2 && splitter->priv->child2) {
        ddb_splitter_remove (GTK_CONTAINER (splitter), splitter->priv->child2);
    }
}

void
ddb_splitter_remove_c1 (DdbSplitter *splitter)
{
    ddb_splitter_remove_child (splitter, 1);
}

void
ddb_splitter_remove_c2 (DdbSplitter *splitter)
{
    ddb_splitter_remove_child (splitter, 2);
}

/**
 * ddb_splitter_get_size_mode:
 * @splitter : a #DdbSplitter.
 *
 * Returns the size mode of the splitter
 *
 * Returns: the size mode of @splitter.
 **/
DdbSplitterSizeMode
ddb_splitter_get_size_mode (const DdbSplitter *splitter)
{
    g_return_val_if_fail (DDB_IS_SPLITTER (splitter), DDB_SPLITTER_SIZE_MODE_PROP);
    return splitter->priv->size_mode;
}

/**
 * ddb_splitter_set_size_mode:
 * @splitter    : a #DdbSplitter.
 * @size_mode : The size_mode of the splitter.
 *
 * Sets the size_mode of the @splitter
 **/
void
ddb_splitter_set_size_mode (DdbSplitter *splitter, DdbSplitterSizeMode size_mode)
{
    g_return_if_fail (DDB_IS_SPLITTER (splitter));

    if (G_LIKELY (splitter->priv->size_mode != size_mode))
    {
        splitter->priv->size_mode = size_mode;
        gtk_widget_queue_resize (GTK_WIDGET (splitter));
        g_object_notify (G_OBJECT (splitter), "size_mode");
    }
}

/**
 * ddb_splitter_get_orientation:
 * @splitter : a #DdbSplitter.
 *
 * Returns the orientation of the splitter
 *
 * Returns: the orientation of @splitter.
 **/
GtkOrientation
ddb_splitter_get_orientation (const DdbSplitter *splitter)
{
    g_return_val_if_fail (DDB_IS_SPLITTER (splitter), GTK_ORIENTATION_HORIZONTAL);
    return splitter->priv->orientation;
}

/**
 * ddb_splitter_set_orientation:
 * @splitter    : a #DdbSplitter.
 * @orientation : The orientation of the splitter.
 *
 * Sets the orientation of the @splitter
 **/
void
ddb_splitter_set_orientation (DdbSplitter *splitter, GtkOrientation orientation)
{
    g_return_if_fail (DDB_IS_SPLITTER (splitter));

    if (G_LIKELY (splitter->priv->orientation != orientation))
    {
        splitter->priv->orientation = orientation;
        gtk_widget_queue_resize (GTK_WIDGET (splitter));
        g_object_notify (G_OBJECT (splitter), "orientation");
    }
}

/**
 * ddb_splitter_get_proportion:
 * @splitter : a #DdbSplitter.
 *
 * Returns the proportion of the splitter
 *
 * Returns: the proportion of the @splitter.
 **/
gfloat
ddb_splitter_get_proportion (const DdbSplitter *splitter)
{
    g_return_val_if_fail (DDB_IS_SPLITTER (splitter), 0.f);
    return splitter->priv->proportion;
}

/**
 * ddb_splitter_set_proportion:
 * @splitter    : a #DdbSplitter.
 * @proportion : The proportion how the child should be arranged.
 *
 * Sets the proportion of the @splitter
 **/
void
ddb_splitter_set_proportion (DdbSplitter *splitter, gfloat proportion)
{
    g_return_if_fail (DDB_IS_SPLITTER (splitter));

    if (splitter->priv->size_mode == DDB_SPLITTER_SIZE_MODE_PROP
           && G_LIKELY (splitter->priv->proportion != proportion))
    {
        splitter->priv->proportion = proportion;
        gtk_widget_queue_resize (GTK_WIDGET (splitter));
        g_object_notify (G_OBJECT (splitter), "proportion");
    }
}

/* Return a new PSquare cast to a GtkWidget */
GtkWidget *
ddb_splitter_new(GtkOrientation orientation)
{
    return GTK_WIDGET (g_object_new (ddb_splitter_get_type (), "orientation", orientation, NULL));
}

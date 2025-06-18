/*
 * DriveAssistify: a utility for managing disk operations.
 *
 * Version: 1.7
 * 
 * Copyright (C) 2024–2025 Maksym Nazar.
 * Created with the assistance of ChatGPT and Perplexity.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <vte/vte.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pango/pango.h>
#if defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#endif

enum {
    COL_NAME = 0,
    COL_SIZE,
    COL_TYPE,
    COL_FSTYPE,
    COL_MOUNTPOINT,
    COL_UUID,
    COL_MODEL,
    COL_ROW_COLOR,
    COL_FONT_COLOR,
    COL_WEIGHT,
    NUM_COLS
};

void run_command(GtkTreeView *tree_view, const gchar *command_template);
void on_device_info_activate(GtkWidget *menuitem, gpointer user_data);
void on_smartctl_activate(GtkWidget *menuitem, gpointer user_data);
void on_show_disk_areas_activate(GtkWidget *menuitem, gpointer user_data);
void on_create_fs_clicked(GtkWidget *button, gpointer user_data);
void on_e2fsck_activate(GtkWidget *menuitem, gpointer user_data);
void on_ext_repair_deep_activate(GtkWidget *menuitem, gpointer user_data);
void on_ntfsfix_activate(GtkWidget *menuitem, gpointer user_data);
void on_ntfsresize_activate(GtkWidget *menuitem, gpointer user_data);
void on_diskscan_activate(GtkWidget *menuitem, gpointer user_data);
void on_mount_activate(GtkWidget *menuitem, gpointer user_data);
void on_umount_activate(GtkWidget *menuitem, gpointer user_data);
void on_umount_f_activate(GtkWidget *menuitem, gpointer user_data);
void on_umount_l_activate(GtkWidget *menuitem, gpointer user_data);
void on_rename_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_partition_table_activate(GtkWidget *menuitem, gpointer user_data);
void on_mkfs_activate(GtkWidget *menuitem, gpointer user_data);
void on_resize_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_grub_install_activate(GtkWidget *menuitem, gpointer user_data);
void on_toggle_boot_flag_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_copy_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_restore_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_delete_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_shred_fs_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_erase_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_multiple_erase_activate(GtkWidget *menuitem, gpointer user_data);
void clean_string(char *str);
void show_disk_list(GtkWidget *widget, gpointer tree_view);
void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void create_termsofuse_window(GtkWidget *parent);
void on_refresh_button_clicked(GtkWidget *button, gpointer user_data);
void on_exit_activate(GtkWidget *menuitem, gpointer user_data);
void on_terms_of_use_activate(GtkWidget *menuitem, gpointer user_data);
void show_command_dialog(GtkWindow *parent, const gchar *title, const gchar *command);
void show_large_text_dialog(GtkWindow *parent, const gchar *title, const gchar *text);
gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

void set_row_color_based_on_type(GtkTreeViewColumn *col,
                                 GtkCellRenderer *renderer,
                                 GtkTreeModel *model,
                                 GtkTreeIter *iter,
                                 gpointer data)
{
    gchar *type = NULL, *name = NULL, *row_color = NULL;
    gtk_tree_model_get(model, iter, COL_TYPE, &type, COL_NAME, &name, COL_ROW_COLOR, &row_color, -1);

    g_object_set(renderer, "text", name, "background", row_color, NULL);

    if (g_strcmp0(type, "disk") == 0) {
        g_object_set(renderer, "foreground", "#0057ae", "weight", PANGO_WEIGHT_BOLD, NULL);
    } else {
        g_object_set(renderer, "foreground", "#222222", "weight", PANGO_WEIGHT_NORMAL, NULL);
    }
    g_free(type);
    g_free(name);
    g_free(row_color);
}

gchar *get_disk_from_partition(const gchar *partition) {
    if (g_str_has_prefix(partition, "nvme") && g_strrstr(partition, "p")) {
        gchar *p = g_strrstr(partition, "p");
        if (p && isdigit(*(p+1))) {
            return g_strndup(partition, p - partition);
        }
    }
    if (g_str_has_prefix(partition, "mmcblk") && g_strrstr(partition, "p")) {
        gchar *p = g_strrstr(partition, "p");
        if (p && isdigit(*(p+1))) {
            return g_strndup(partition, p - partition);
        }
    }
    for (const char *p = partition; *p; ++p) {
        if (isdigit(*p)) return g_strndup(partition, p - partition);
    }
    return g_strdup(partition);
}

static void on_terminal_child_exited(VteTerminal *terminal, gint status, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_set_title(window, "Command Finished");
}

void run_command(GtkTreeView *tree_view, const gchar *command_template) {
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name;

    selection = gtk_tree_view_get_selection(tree_view);
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *quoted_path = g_shell_quote(device_path);
        gchar *command = g_strdup_printf(command_template, quoted_path);

        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Command Terminal");
        gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
        gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

        GtkWidget *terminal = vte_terminal_new();
        gtk_container_add(GTK_CONTAINER(window), terminal);

        g_signal_connect(terminal, "child-exited", G_CALLBACK(on_terminal_child_exited), window);

        vte_terminal_spawn_async(VTE_TERMINAL(terminal), VTE_PTY_DEFAULT,
                                 NULL, (char *[]){"/bin/sh", "-c", command, NULL},
                                 NULL, 0, NULL, NULL, NULL, -1, NULL, NULL, NULL);

        g_free(command);
        g_free(quoted_path);
        g_free(device_path);
        g_free(disk_name);

        gtk_widget_show_all(window);
    }
}

static gchar *get_base_device(const gchar *dev) {
    if (g_str_has_prefix(dev, "nvme") && g_strrstr(dev, "p")) {
        gchar *p = g_strrstr(dev, "p");
        if (p && isdigit(*(p+1))) return g_strndup(dev, p - dev);
    }
    if (g_str_has_prefix(dev, "mmcblk") && g_strrstr(dev, "p")) {
        gchar *p = g_strrstr(dev, "p");
        if (p && isdigit(*(p+1))) return g_strndup(dev, p - dev);
    }
    for (const char *p = dev; *p; ++p) {
        if (isdigit(*p)) return g_strndup(dev, p - dev);
    }
    return g_strdup(dev);
}

void on_device_info_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);

        gchar *base_device = get_base_device(disk_name);
        gchar *fdisk_cmd = g_strdup_printf("LC_ALL=C sudo fdisk -l /dev/%s", base_device);
        FILE *fdisk_fp = popen(fdisk_cmd, "r");
        char fdisk_line[1024];
        char start_sector[64] = "";
        char end_sector[64] = "";
        char total_sectors[64] = "";
        gboolean found = FALSE;
        while (fdisk_fp && fgets(fdisk_line, sizeof(fdisk_line), fdisk_fp)) {
            gchar *trim = g_strstrip(fdisk_line);
            if (!found && g_str_has_prefix(trim, "Disk ") && strstr(trim, base_device) && strstr(trim, "sectors")) {
                char *p = strstr(trim, "sectors");
                if (p) {
                    while (p > trim && !isdigit(*(p-1))) --p;
                    char *end = p;
                    while (p > trim && isdigit(*(p-1))) --p;
                    strncpy(total_sectors, p, end - p);
                    total_sectors[end - p] = '\0';
                }
                found = TRUE;
                continue;
            }
            if (g_str_has_prefix(trim, device_path)) {
                char dev[128], start[64], end[64], sectors[64];
                int n = sscanf(trim, "%127s %63s %63s %63s", dev, start, end, sectors);
                if (n >= 4) {
                    strcpy(start_sector, start);
                    strcpy(end_sector, end);
                    strcpy(total_sectors, sectors);
                    found = TRUE;
                    break;
                }
            }
        }
        if (fdisk_fp) pclose(fdisk_fp);
        g_free(fdisk_cmd);
        g_free(base_device);

        GString *sector_info = g_string_new("");
        if (strlen(start_sector) > 0 && strlen(end_sector) > 0 && strlen(total_sectors) > 0) {
            g_string_append_printf(sector_info,
                "=== SECTOR INFO ===\nStart sector: %s\nEnd sector: %s\nTotal sectors: %s\n",
                start_sector, end_sector, total_sectors);
        } else if (strlen(total_sectors) > 0) {
            g_string_append_printf(sector_info,
                "=== SECTOR INFO ===\nTotal sectors: %s\n", total_sectors);
        }

        gchar *command = g_strdup_printf(
            "echo '=== LSBLK ==='; lsblk -f %s; "
            "echo ''; echo '=== FDISK ==='; sudo fdisk -l %s; "
            "echo ''; echo '=== PARTED ==='; sudo parted %s print; "
            "echo ''; echo '=== BLKID ==='; sudo blkid %s",
            device_path, device_path, device_path, device_path
        );

        FILE *fp = popen(command, "r");
        if (fp) {
            GString *output = g_string_new(NULL);
            char line_buffer[4096];
            while (fgets(line_buffer, sizeof(line_buffer), fp)) {
                g_string_append(output, line_buffer);
            }
            pclose(fp);

            gchar *cleaned = g_regex_replace(
                g_regex_new("\\n{3,}", 0, 0, NULL),
                output->str, -1, 0, "\n\n", 0, NULL
            );

            gchar *final_text;
            if (strlen(sector_info->str) > 0) {
                int len = strlen(cleaned);
                while (len > 0 && cleaned[len-1] == '\n') --len;
                gchar *trimmed = g_strndup(cleaned, len);
                final_text = g_strconcat(trimmed, "\n\n", sector_info->str, NULL);
                g_free(trimmed);
            } else {
                final_text = g_strdup(cleaned);
            }

            GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title(GTK_WINDOW(window), "Device Information");
            gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
            gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
            gtk_container_set_border_width(GTK_CONTAINER(window), 10);

            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_container_add(GTK_CONTAINER(window), box);

            GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
            gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);
            gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);

            GtkWidget *text_view = gtk_text_view_new();
            gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
            gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), TRUE);
            gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
            gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 5);
            gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);
            gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 5);
            gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view), 5);

            GtkCssProvider *provider = gtk_css_provider_new();
            gtk_css_provider_load_from_data(provider,
                "textview { font-family: 'monospace'; font-size: 9.5pt; }", -1, NULL);
            GtkStyleContext *context = gtk_widget_get_style_context(text_view);
            gtk_style_context_add_provider(context,
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_USER);
            g_object_unref(provider);

            gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

            GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(text_buffer, final_text, -1);

            gtk_widget_show_all(window);

            g_free(final_text);
            g_free(cleaned);
            g_string_free(output, TRUE);
        }

        g_string_free(sector_info, TRUE);
        g_free(command);
        g_free(device_path);
        g_free(disk_name);
    }
}

void on_smartctl_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);
        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *command = g_strdup_printf("smartctl -x %s; smartctl -H %s", device_path, device_path);

        FILE *fp = popen(command, "r");
        if (fp) {
            GString *output = g_string_new(NULL);
            char buffer[4096];
            while (fgets(buffer, sizeof(buffer), fp)) {
                g_string_append(output, buffer);
            }
            pclose(fp);

            GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title(GTK_WINDOW(window), "SMART Information");
            gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
            gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
            gtk_container_set_border_width(GTK_CONTAINER(window), 10);

            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
            gtk_container_add(GTK_CONTAINER(window), box);

            GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
            gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);
            gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);

            GtkWidget *text_view = gtk_text_view_new();
            gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
            gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), TRUE);
            gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);
            gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 5);
            gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);
            gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 5);
            gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view), 5);

            GtkCssProvider *provider = gtk_css_provider_new();
            gtk_css_provider_load_from_data(provider,
                "textview { font-family: 'monospace'; font-size: 9.5pt; }", -1, NULL);
            GtkStyleContext *context = gtk_widget_get_style_context(text_view);
            gtk_style_context_add_provider(context,
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_USER);
            g_object_unref(provider);

            gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

            GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            gtk_text_buffer_set_text(text_buffer, output->str, -1);

            gtk_widget_show_all(window);
            g_string_free(output, TRUE);
        }

        g_free(command);
        g_free(device_path);
        g_free(disk_name);
    }
}

void on_show_disk_areas_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        if (g_regex_match_simple("^[a-zA-Z]+[0-9]+$", disk_name, 0, 0)) {
            GtkWidget *warn = gtk_message_dialog_new(
                NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                "This function is intended only for entire disks (e.g., /dev/sdb), not for partitions (e.g., /dev/sdb1)."
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);
            g_free(disk_name);
            return;
        }

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *command = g_strdup_printf("parted %s unit MiB print free", device_path);

        FILE *fp = popen(command, "r");
        if (fp) {
            GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title(GTK_WINDOW(window), g_strdup_printf("Disk Areas (%s)", device_path));
            gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);
            gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

            GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
            gtk_container_add(GTK_CONTAINER(window), box);

            GtkWidget *tree = gtk_tree_view_new();
            GtkListStore *store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
            gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
            g_object_unref(store);

            const char *titles[] = {"Start", "End", "Size", "Type", "File system"};
            for (int i = 0; i < 5; ++i) {
                GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
                GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
                gtk_tree_view_append_column(GTK_TREE_VIEW(tree), col);
            }

            char line[512];
            gboolean in_table = FALSE;
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "Number") && strstr(line, "File system")) {
                    in_table = TRUE;
                    continue;
                }
                if (!in_table) continue;
                gchar *trim = g_strstrip(line);
                if (strlen(trim) == 0) continue;
                if (strstr(trim, "Free Space")) {
                    gchar start[32] = "", end[32] = "", size[32] = "", type[32] = "Free Space", fs[32] = "";
                    sscanf(trim, "%31s %31s %31s", start, end, size);
                    GtkTreeIter row;
                    gtk_list_store_append(store, &row);
                    gtk_list_store_set(store, &row, 0, start, 1, end, 2, size, 3, type, 4, fs, -1);
                } else {
                    gchar num[16] = "", start[32] = "", end[32] = "", size[32] = "", type[32] = "", fs[32] = "";
                    int parsed = sscanf(trim, "%15s %31s %31s %31s %31s %31s", num, start, end, size, type, fs);
                    if (parsed >= 4) {
                        gchar *part_dev = g_strdup_printf("%s%s", device_path, num);
                        if (parsed < 6 || strlen(fs) == 0 || g_strcmp0(fs, "unknown") == 0) {
                            gchar *blkid_cmd = g_strdup_printf("blkid -o value -s TYPE %s 2>/dev/null", part_dev);
                            FILE *blkid_fp = popen(blkid_cmd, "r");
                            if (blkid_fp) {
                                char fs_type[64] = "";
                                if (fgets(fs_type, sizeof(fs_type), blkid_fp)) {
                                    g_strstrip(fs_type);
                                    if (strlen(fs_type) > 0)
                                        strncpy(fs, fs_type, sizeof(fs)-1);
                                }
                                pclose(blkid_fp);
                            }
                            g_free(blkid_cmd);
                        }
                        GtkTreeIter row;
                        gtk_list_store_append(store, &row);
                        gtk_list_store_set(store, &row, 0, start, 1, end, 2, size, 3, type, 4, fs, -1);
                        g_free(part_dev);
                    }
                }
            }
            pclose(fp);

            GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
            gtk_container_add(GTK_CONTAINER(scrolled), tree);
            gtk_box_pack_start(GTK_BOX(box), scrolled, TRUE, TRUE, 0);

            GtkWidget *create_fs_btn = gtk_button_new_with_label("Create Filesystem on Selected Area");
            gtk_box_pack_start(GTK_BOX(box), create_fs_btn, FALSE, FALSE, 0);

            g_signal_connect(create_fs_btn, "clicked", G_CALLBACK(on_create_fs_clicked), tree);

            gtk_widget_show_all(window);
        }

        g_free(command);
        g_free(device_path);
        g_free(disk_name);
    }
}

void on_create_fs_clicked(GtkWidget *button, gpointer user_data) {
    GtkTreeView *tree = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model = gtk_tree_view_get_model(tree);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree);
    GtkTreeIter iter;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *err = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Please select an area in the list.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        return;
    }

    gchar *start = NULL, *end = NULL, *type = NULL;
    gtk_tree_model_get(model, &iter, 0, &start, 1, &end, 3, &type, -1);

    if (!type || g_strcmp0(type, "Free Space") != 0) {
        GtkWidget *err = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Please select a 'Free Space' area to create a new filesystem.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        g_free(start); g_free(end); if (type) g_free(type);
        return;
    }

    GtkWidget *parent_window = gtk_widget_get_toplevel(GTK_WIDGET(tree));
    gchar *disk_path = NULL;
    if (GTK_IS_WINDOW(parent_window)) {
        const gchar *title = gtk_window_get_title(GTK_WINDOW(parent_window));
        const gchar *open = strchr(title, '(');
        const gchar *close = strchr(title, ')');
        if (open && close && close > open) {
            disk_path = g_strndup(open + 1, close - open - 1);
        }
    }
    if (!disk_path) {
        GtkWidget *err = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "Cannot determine disk device path.");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        g_free(start); g_free(end); if (type) g_free(type);
        return;
    }

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Create Filesystem",
        NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Create", GTK_RESPONSE_ACCEPT,
        NULL
    );

    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 95);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext4");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext3");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ntfs");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "exfat");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fat32");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new("Select filesystem type:"));
    gtk_container_add(GTK_CONTAINER(content_area), combo);

    GtkWidget *align_check = gtk_check_button_new_with_label(
        "Align partition (start at 1 MiB boundary, recommended)"
    );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(align_check), TRUE);
    gtk_container_add(GTK_CONTAINER(content_area), align_check);

    gtk_widget_show_all(dialog);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const gchar *fs_type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        gboolean align = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(align_check));

        long start_mib = atol(start);
        long end_mib = atol(end);

        if (align) {
            if (start_mib % 1 != 0)
                start_mib = (start_mib / 1) + 1;
        }

        gchar *mkpart_cmd = g_strdup_printf(
            "sudo parted -s %s mkpart primary %s %ldMiB %ldMiB",
            disk_path, fs_type, start_mib, end_mib
        );

        gchar *lsblk_cmd = g_strdup_printf(
            "lsblk -lnpo NAME %s | grep -v '^%s$' | tail -n 1",
            disk_path, disk_path
        );

        gchar *mkfs_cmd = NULL;
        if (g_strcmp0(fs_type, "ext4") == 0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ext4 -F $(%s)", lsblk_cmd);
        else if (g_strcmp0(fs_type, "ext3") == 0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ext3 -F $(%s)", lsblk_cmd);
        else if (g_strcmp0(fs_type, "ext2") == 0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ext2 -F $(%s)", lsblk_cmd);
        else if (g_strcmp0(fs_type, "ntfs") == 0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ntfs -F $(%s)", lsblk_cmd);
        else if (g_strcmp0(fs_type, "exfat") == 0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.exfat $(%s)", lsblk_cmd);
        else if (g_strcmp0(fs_type, "fat32") == 0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.fat -F 32 $(%s)", lsblk_cmd);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
            "WARNING: This will create a new partition and destroy any data in the selected area!\n\nFilesystem type: %s\n\nAre you sure you want to continue?",
            fs_type
        );
        gint warn_response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (warn_response == GTK_RESPONSE_OK) {
            gchar *full_cmd = g_strdup_printf(
                "%s && (sudo partprobe %s || sudo blockdev --rereadpt %s) && sleep 1 && %s",
                mkpart_cmd, disk_path, disk_path, mkfs_cmd
            );
            run_command(tree, full_cmd);
            g_free(full_cmd);
        }

        g_free(mkpart_cmd);
        g_free(lsblk_cmd);
        g_free(mkfs_cmd);
    }

    gtk_widget_destroy(dialog);
    g_free(start); g_free(end); if (type) g_free(type);
    if (disk_path) g_free(disk_path);
}

void on_e2fsck_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "e2fsck -f -y -v %s");
}

void on_ext_repair_deep_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        gchar *cmd = g_strdup_printf("mke2fs -n %s | grep -oE '[0-9]+' | tr '\\n' ' '", device_path);
        FILE *fp = popen(cmd, "r");
        gchar superblocks[1024] = {0};
        if (fp) {
            fgets(superblocks, sizeof(superblocks)-1, fp);
            pclose(fp);
        }
        g_free(cmd);

        gchar *sb_tokens = g_strdup(superblocks);
        gchar *saveptr = NULL;
        gchar *tokens[10] = {0};
        int count = 0;
        gchar *token = strtok_r(sb_tokens, " ", &saveptr);
        while (token && count < 10) {
            tokens[count++] = token;
            token = strtok_r(NULL, " ", &saveptr);
        }

        if (count >= 3) {
            gchar *message = g_strdup_printf(
                "WARNING: This operation will attempt advanced recovery of the EXT2/3/4 filesystem using a selected backup superblock.\n\n"
                "Target: %s\n\n"
                "Choose a backup superblock to use for recovery:\n"
                "2) %s\n"
                "3) %s\n\n"
                "You should try the 2nd first (usually %s), then the 3rd if needed.\n\n"
                "Are you sure you want to continue?",
                device_path, tokens[1], tokens[2], tokens[1]
            );
            GtkWidget *dialog = gtk_dialog_new_with_buttons(
                "Select Backup Superblock",
                NULL,
                GTK_DIALOG_MODAL,
                "_Cancel", GTK_RESPONSE_CANCEL,
                "_Use 2nd", 2,
                "_Use 3rd", 3,
                NULL
            );
            GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
            GtkWidget *label = gtk_label_new(message);
            gtk_label_set_xalign(GTK_LABEL(label), 0.0);
            gtk_box_pack_start(GTK_BOX(content_area), label, FALSE, FALSE, 0);
            gtk_widget_show_all(dialog);

            gint response = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            g_free(message);

            gchar *sb_to_use = NULL;
            if (response == 2) sb_to_use = tokens[1];
            else if (response == 3) sb_to_use = tokens[2];

            if (sb_to_use) {
            gchar *repair_cmd = g_strdup_printf(
                "e2fsck -b %s -y %s; "
                "echo ''; echo 'Now running full filesystem check...'; "
                "e2fsck -f -y -v %s",
                sb_to_use, device_path, device_path
            );
                run_command(tree_view, repair_cmd);
                g_free(repair_cmd);
            }
        } else {
            GtkWidget *err = gtk_message_dialog_new(
                NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Could not find enough backup superblocks for this partition."
            );
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        }

        g_free(sb_tokens);
        g_free(device_path);
        g_free(partition_name);
    }
}

void on_ntfsfix_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "ntfsfix %s");
}

void on_ntfsresize_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "ntfsresize -P -i -f -v %s");
}

void on_diskscan_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "diskscan %s");
}

void on_mount_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *mount_point = g_strdup_printf("/mnt/%s", disk_name);
        gchar *quoted_device_path = g_shell_quote(device_path);
        gchar *quoted_mount_point = g_shell_quote(mount_point);

        gchar *mkdir_command = g_strdup_printf("mkdir -p %s", quoted_mount_point);
        system(mkdir_command);
        g_free(mkdir_command);

        gchar *command = g_strdup_printf("if [ -b %s ]; then mount %s %s && echo 'Mounted successfully'; else echo 'Device not found'; fi", quoted_device_path, quoted_device_path, quoted_mount_point);
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Command Terminal");
        gtk_window_set_default_size(GTK_WINDOW(window), 900, 400);

        GtkWidget *terminal = vte_terminal_new();
        gtk_container_add(GTK_CONTAINER(window), terminal);

        vte_terminal_spawn_async(VTE_TERMINAL(terminal), VTE_PTY_DEFAULT,
                                 NULL, (char *[]){"/bin/sh", "-c", command, NULL},
                                 NULL, 0, NULL, NULL, NULL, -1, NULL, NULL, NULL);

        g_free(command);
        g_free(quoted_device_path);
        g_free(quoted_mount_point);
        g_free(device_path);
        g_free(mount_point);
        g_free(disk_name);

        gtk_widget_show_all(window);
    }
}

void on_umount_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "umount %s && echo 'Unmounted successfully'");
}

void on_umount_l_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "umount -l %s && echo 'Lazy unmount successful'");
}

void on_umount_f_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command(GTK_TREE_VIEW(user_data), "umount -f %s && echo 'Forced unmount successful'");
}

void on_rename_partition_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *fstype = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_FSTYPE, &fstype, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Rename Partition",
            NULL,
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Rename", GTK_RESPONSE_ACCEPT,
            NULL
        );
        
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 60);
        
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(entry), 16);
        gtk_entry_set_width_chars(GTK_ENTRY(entry), 24);
        
        gtk_box_pack_start(GTK_BOX(content_area), entry, FALSE, FALSE, 0);
        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gchar *new_label = NULL;
        if (response == GTK_RESPONSE_ACCEPT) {
            new_label = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
        }
        gtk_widget_destroy(dialog);

        if (new_label && strlen(new_label) > 0) {
            gchar *cmd = NULL;
            if (g_strcmp0(fstype, "ext4") == 0 || g_strcmp0(fstype, "ext3") == 0 || g_strcmp0(fstype, "ext2") == 0) {
                cmd = g_strdup_printf("e2label %s '%s'; echo 'Label changed (ext2/3/4)'", device_path, new_label);
            } else if (g_strcmp0(fstype, "ntfs") == 0) {
                cmd = g_strdup_printf("ntfslabel %s '%s'; echo 'Label changed (NTFS)'", device_path, new_label);
            } else if (g_strcmp0(fstype, "exfat") == 0) {
                cmd = g_strdup_printf("exfatlabel %s '%s'; echo 'Label changed (exFAT)'", device_path, new_label);
            } else if (g_strcmp0(fstype, "vfat") == 0 || g_strcmp0(fstype, "fat32") == 0 || g_strcmp0(fstype, "fat") == 0) {
                cmd = g_strdup_printf("fatlabel %s '%s'; echo 'Label changed (FAT32/FAT)'", device_path, new_label);
            } else if (g_strcmp0(fstype, "xfs") == 0) {
                cmd = g_strdup_printf("xfs_admin -L '%s' %s; echo 'Label changed (XFS)'", new_label, device_path);
            } else if (g_strcmp0(fstype, "btrfs") == 0) {
                cmd = g_strdup_printf("btrfs filesystem label %s '%s'; echo 'Label changed (Btrfs)'", device_path, new_label);
            } else {
                cmd = g_strdup_printf("echo 'Unsupported or unknown filesystem: %s'", fstype ? fstype : "unknown");
            }

            run_command(tree_view, cmd);
            g_free(cmd);
            g_free(new_label);
        }

        g_free(device_path);
        g_free(partition_name);
        g_free(fstype);
    }
}

void on_partition_table_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;
    gchar *type = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, COL_TYPE, &type, -1);

        if (g_strcmp0(type, "disk") != 0) {
            GtkWidget *err = gtk_message_dialog_new(
                NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Please select a whole disk (not a partition) for this operation."
            );
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            g_free(type);
            g_free(disk_name);
            return;
        }
        g_free(type);

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Choose Partition Table Type",
            NULL,
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Create", GTK_RESPONSE_OK,
            NULL
        );
        gtk_window_set_default_size(GTK_WINDOW(dialog), 350, 120);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "MBR (msdos)");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "GPT");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
        gtk_container_add(GTK_CONTAINER(content_area), combo);
        gtk_widget_show_all(dialog);

        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        int table_type = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
        gtk_widget_destroy(dialog);

        if (result == GTK_RESPONSE_OK) {
            gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
            gchar *quoted_device = g_shell_quote(device_path);

            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK_CANCEL,
                "WARNING: Creating a new partition table will destroy all data on %s!\n\nContinue?",
                device_path
            );
            gint resp = gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            if (resp == GTK_RESPONSE_OK) {
                const char *table = (table_type == 0) ? "msdos" : "gpt";
                gchar *fdisk_cmd = (table_type == 0)
                    ? g_strdup_printf("echo -e 'o\\nw\\n' | sudo fdisk %s", quoted_device)
                    : g_strdup_printf("echo -e 'g\\nw\\n' | sudo fdisk %s", quoted_device);

                gchar *command = g_strdup_printf(
                    "for part in $(lsblk -ln -o NAME %s | grep -v ^$(basename %s)$); do umount /dev/$part 2>/dev/null; done; "
                    "(sudo parted -s %s mklabel %s || %s) && "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s) && "
                    "echo 'Partition table (%s) created and partition table re-read.'",
                    quoted_device, quoted_device, quoted_device, table, fdisk_cmd, quoted_device, quoted_device, table
                );
                run_command(tree_view, command);
                g_free(fdisk_cmd);
                g_free(command);
            }
            g_free(quoted_device);
            g_free(device_path);
        }
        g_free(disk_name);
    }
}

void on_mkfs_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;
    gchar *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_MOUNTPOINT, &mountpoint, -1);
        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Create Filesystem",
            NULL,
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Create", GTK_RESPONSE_ACCEPT,
            NULL
        );
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 80);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext4");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext3");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext2");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ntfs");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "exfat");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fat32");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
        gtk_container_add(GTK_CONTAINER(content_area), gtk_label_new("Select filesystem type:"));
        gtk_container_add(GTK_CONTAINER(content_area), combo);
        gtk_widget_show_all(dialog);

        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_ACCEPT) {
            const gchar *fs_type = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
            gchar *mkfs_cmd = NULL;

            if (g_strcmp0(fs_type, "ext4") == 0)
                mkfs_cmd = g_strdup_printf("mkfs.ext4 -F %s", device_path);
            else if (g_strcmp0(fs_type, "ext3") == 0)
                mkfs_cmd = g_strdup_printf("mkfs.ext3 -F %s", device_path);
            else if (g_strcmp0(fs_type, "ext2") == 0)
                mkfs_cmd = g_strdup_printf("mkfs.ext2 -F %s", device_path);
            else if (g_strcmp0(fs_type, "ntfs") == 0)
                mkfs_cmd = g_strdup_printf("mkfs.ntfs -F %s", device_path);
            else if (g_strcmp0(fs_type, "exfat") == 0)
                mkfs_cmd = g_strdup_printf("mkfs.exfat %s", device_path);
            else if (g_strcmp0(fs_type, "fat32") == 0)
                mkfs_cmd = g_strdup_printf("mkfs.fat -F 32 %s", device_path);

            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK_CANCEL,
                "WARNING: This will destroy all data on %s!\n\nFilesystem type: %s\n\nAre you sure you want to continue?",
                device_path, fs_type
            );
            gint warn_response = gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            if (warn_response == GTK_RESPONSE_OK) {
                gchar *command = NULL;
                if (mountpoint && strlen(mountpoint) > 0 && strcmp(mountpoint, "N/A") != 0 && strcmp(mountpoint, "-") != 0) {
                    command = g_strdup_printf(
                        "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                        "echo 'Formatting %s as %s...'; sudo %s; "
                        "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                        "echo 'Filesystem created and partition table re-read.'",
                        device_path, device_path, device_path, fs_type, mkfs_cmd, device_path, device_path
                    );
                } else {
                    command = g_strdup_printf(
                        "echo 'Formatting %s as %s...'; sudo %s; "
                        "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                        "echo 'Filesystem created and partition table re-read.'",
                        device_path, fs_type, mkfs_cmd, device_path, device_path
                    );
                }
                run_command(tree_view, command);
                g_free(command);
            }
            g_free(mkfs_cmd);
        }
        gtk_widget_destroy(dialog);
        g_free(device_path);
        g_free(partition_name);
        if (mountpoint) g_free(mountpoint);
    }
}

typedef struct {
    GtkWidget *label_free;
    int orig_free;
    int orig_size;
    GtkWidget *entry_start;
    GtkWidget *entry_end;
    GtkWidget *entry_mib;
    int sector_size;
} ResizeWidgets;

static gboolean is_updating = FALSE;

static void on_entry_mib_changed(GtkEditable *editable, gpointer user_data) {
    if (is_updating) return;
    is_updating = TRUE;

    ResizeWidgets *widgets = (ResizeWidgets*)user_data;
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(editable));
    int input = atoi(text);
    int left = (widgets->orig_size + widgets->orig_free) - input;
    if (left < 0) left = 0;
    gchar label[128];
    g_snprintf(label, sizeof(label), "Free space after resize: %d MiB", left);
    gtk_label_set_text(GTK_LABEL(widgets->label_free), label);

    const gchar *start_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_start));
    if (start_text && strlen(start_text) > 0) {
        int start = atoi(start_text);
        int sectors = (input * 1024 * 1024) / widgets->sector_size;
        int end = start + sectors - 1;
        if (start >= 0 && sectors > 0 && end >= start) {
            gchar end_str[32];
            g_snprintf(end_str, sizeof(end_str), "%d", end);
            gtk_entry_set_text(GTK_ENTRY(widgets->entry_end), end_str);
        } else {
            gtk_entry_set_text(GTK_ENTRY(widgets->entry_end), "");
        }
    } else {
        gtk_entry_set_text(GTK_ENTRY(widgets->entry_end), "");
    }

    is_updating = FALSE;
}

static void on_entry_start_changed(GtkEditable *editable, gpointer user_data) {
    if (is_updating) return;
    is_updating = TRUE;

    ResizeWidgets *widgets = (ResizeWidgets*)user_data;
    const gchar *start_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_start));
    const gchar *mib_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_mib));
    int start = atoi(start_text);
    int input = atoi(mib_text);

    if (input > 0 && widgets->sector_size > 0) {
        int sectors = (input * 1024 * 1024) / widgets->sector_size;
        int end = start + sectors - 1;
        if (start >= 0 && sectors > 0 && end >= start) {
            gchar end_str[32];
            g_snprintf(end_str, sizeof(end_str), "%d", end);
            gtk_entry_set_text(GTK_ENTRY(widgets->entry_end), end_str);
        } else {
            gtk_entry_set_text(GTK_ENTRY(widgets->entry_end), "");
        }
    } else {
        gtk_entry_set_text(GTK_ENTRY(widgets->entry_end), "");
    }

    is_updating = FALSE;
}

static void on_entry_end_changed(GtkEditable *editable, gpointer user_data) {
    if (is_updating) return;
    is_updating = TRUE;

    ResizeWidgets *widgets = (ResizeWidgets*)user_data;
    const gchar *start_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_start));
    const gchar *end_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_end));
    int start = atoi(start_text);
    int end = atoi(end_text);
    if (end > start && widgets->sector_size > 0) {
        int sectors = end - start + 1;
        int mib = (sectors * widgets->sector_size) / (1024 * 1024);
        gchar mib_str[32];
        g_snprintf(mib_str, sizeof(mib_str), "%d", mib);
        gtk_entry_set_text(GTK_ENTRY(widgets->entry_mib), mib_str);
    }

    is_updating = FALSE;
}

static void on_resize_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void on_resize_dialog_destroy(GtkWidget *dialog, gpointer user_data);

void on_resize_partition_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *fstype = NULL, *size = NULL;

    gchar *device_path = NULL;
    GtkWidget *main_window = NULL;
    GtkWidget *dialog = NULL;
    GtkWidget *content_area = NULL;
    GtkWidget *label1 = NULL;
    GtkWidget *label2 = NULL;
    GtkWidget *entry_mib = NULL;
    GtkWidget *entry_start = NULL;
    GtkWidget *entry_end = NULL;
    gchar *disk_name = NULL;
    gchar *device_disk = NULL;
    gchar *command = NULL;
    GtkWidget *label_free = NULL;
    int max_free = 0;
    int orig_size_mib = 0;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;

    gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_FSTYPE, &fstype, COL_SIZE, &size, -1);
    device_path = g_strdup_printf("/dev/%s", partition_name);

    main_window = gtk_widget_get_toplevel(GTK_WIDGET(tree_view));
    if (!GTK_IS_WINDOW(main_window)) main_window = NULL;

    dialog = gtk_dialog_new_with_buttons(
        "Resize Partition",
        main_window ? GTK_WINDOW(main_window) : NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Resize", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 220);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label1 = gtk_label_new("Enter new partition size in MiB (leave empty to use sectors):");
    entry_mib = gtk_entry_new();
    entry_start = gtk_entry_new();
    entry_end = gtk_entry_new();

    if (size && strlen(size) > 0) {
        double gb = g_ascii_strtod(size, NULL);
        orig_size_mib = (int)(gb * 1024);
        gchar mib_str[32];
        g_snprintf(mib_str, sizeof(mib_str), "%d", orig_size_mib);
        gtk_entry_set_text(GTK_ENTRY(entry_mib), mib_str);
    } else {
        gtk_entry_set_text(GTK_ENTRY(entry_mib), "");
    }

    disk_name = get_disk_from_partition(partition_name);
    device_disk = g_strdup_printf("/dev/%s", disk_name);

    int sector_size = 512;
    gchar *ss_cmd = g_strdup_printf("blockdev --getss %s", device_disk);
    FILE *ss_fp = popen(ss_cmd, "r");
    if (ss_fp) {
        fscanf(ss_fp, "%d", &sector_size);
        pclose(ss_fp);
    }
    g_free(ss_cmd);

    command = g_strdup_printf("LC_ALL=C parted %s unit MiB print free", device_disk);

    FILE *fp = popen(command, "r");
    char line[512];
    gboolean in_table = FALSE;
    while (fp && fgets(line, sizeof(line), fp)) {
        if (strstr(line, "Number") && strstr(line, "File system")) {
            in_table = TRUE;
            continue;
        }
        if (!in_table) continue;
        gchar *trim = g_strstrip(line);
        if (strlen(trim) == 0) continue;
        if (strstr(trim, "Free Space")) {
            gchar start[64], end[64], size_str[64], type[64];
            int parsed = sscanf(trim, "%63s %63s %63s %63[^\n]", start, end, size_str, type);
            if (parsed >= 4) {
                int val = atoi(size_str);
                if (val > max_free) max_free = val;
            }
        }
    }
    if (fp) pclose(fp);
    g_free(command);
    g_free(device_disk);
    g_free(disk_name);

    if (max_free > 0) {
        gchar free_label[64];
        g_snprintf(free_label, sizeof(free_label), "Free space: %d MiB", max_free);
        label_free = gtk_label_new(free_label);
    } else {
        label_free = gtk_label_new("Free space: unknown");
    }

    label2 = gtk_label_new("Or specify first and last sector (optional):");

    gtk_entry_set_text(GTK_ENTRY(entry_start), "2048");

    ResizeWidgets *widgets = g_new0(ResizeWidgets, 1);
    widgets->label_free   = label_free;
    widgets->orig_free    = max_free;
    widgets->orig_size    = orig_size_mib;
    widgets->entry_start  = entry_start;
    widgets->entry_end    = entry_end;
    widgets->entry_mib    = entry_mib;
    widgets->sector_size  = sector_size;

    g_signal_connect(entry_mib,   "changed", G_CALLBACK(on_entry_mib_changed), widgets);
    g_signal_connect(entry_start, "changed", G_CALLBACK(on_entry_start_changed), widgets);
    g_signal_connect(entry_end,   "changed", G_CALLBACK(on_entry_end_changed), widgets);

    on_entry_mib_changed(GTK_EDITABLE(entry_mib), widgets);

    gtk_container_add(GTK_CONTAINER(content_area), label1);
    gtk_container_add(GTK_CONTAINER(content_area), entry_mib);
    gtk_container_add(GTK_CONTAINER(content_area), label_free);
    gtk_container_add(GTK_CONTAINER(content_area), label2);
    gtk_container_add(GTK_CONTAINER(content_area), entry_start);
    gtk_container_add(GTK_CONTAINER(content_area), entry_end);

    GtkWidget *alignment_info = gtk_label_new(
        "For best compatibility and performance, the start sector should be 2048 \nor a multiple of 2048 (1 MiB alignment). This ensures proper partition \nalignment and avoids issues with modern storage devices."
    );
    gtk_widget_set_halign(alignment_info, GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(content_area), alignment_info);

    gtk_widget_show_all(dialog);

    g_object_set_data(G_OBJECT(dialog), "widgets", widgets);
    g_object_set_data(G_OBJECT(dialog), "tree_view", tree_view);
    g_object_set_data(G_OBJECT(dialog), "partition_name", partition_name);
    g_object_set_data(G_OBJECT(dialog), "fstype", fstype);
    g_object_set_data(G_OBJECT(dialog), "size", size);
    g_object_set_data(G_OBJECT(dialog), "device_path", device_path);
    g_object_set_data(G_OBJECT(dialog), "entry_mib", entry_mib);
    g_object_set_data(G_OBJECT(dialog), "entry_start", entry_start);
    g_object_set_data(G_OBJECT(dialog), "entry_end", entry_end);

    g_signal_connect(dialog, "response", G_CALLBACK(on_resize_dialog_response), NULL);
    g_signal_connect(dialog, "destroy", G_CALLBACK(on_resize_dialog_destroy), NULL);
}

static void on_resize_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    ResizeWidgets *widgets = g_object_get_data(G_OBJECT(dialog), "widgets");
    GtkTreeView *tree_view = g_object_get_data(G_OBJECT(dialog), "tree_view");
    gchar *partition_name = g_object_get_data(G_OBJECT(dialog), "partition_name");
    gchar *fstype = g_object_get_data(G_OBJECT(dialog), "fstype");
    gchar *size = g_object_get_data(G_OBJECT(dialog), "size");
    gchar *device_path = g_object_get_data(G_OBJECT(dialog), "device_path");
    GtkWidget *entry_mib = g_object_get_data(G_OBJECT(dialog), "entry_mib");
    GtkWidget *entry_start = g_object_get_data(G_OBJECT(dialog), "entry_start");
    GtkWidget *entry_end = g_object_get_data(G_OBJECT(dialog), "entry_end");

    GtkWidget *main_window = gtk_widget_get_toplevel(GTK_WIDGET(tree_view));
    if (!GTK_IS_WINDOW(main_window)) main_window = NULL;

    if (response_id == GTK_RESPONSE_ACCEPT) {
        const gchar *size_str = gtk_entry_get_text(GTK_ENTRY(entry_mib));
        const gchar *start_sector = gtk_entry_get_text(GTK_ENTRY(entry_start));
        const gchar *end_sector = gtk_entry_get_text(GTK_ENTRY(entry_end));
        gchar *cmd = NULL;
        gboolean use_sectors = (strlen(start_sector) > 0 && strlen(end_sector) > 0);

        if (use_sectors && strlen(size_str) > 0) {
            GtkWidget *warn = gtk_message_dialog_new(
                main_window ? GTK_WINDOW(main_window) : NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                "Both size in MiB and sector values are specified.\n"
                "The program will use the sector values and ignore the size in MiB."
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);
        }

        if (use_sectors) {
            if (g_strcmp0(fstype, "ext4") == 0 || g_strcmp0(fstype, "ext3") == 0 || g_strcmp0(fstype, "ext2") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition to sectors...'; sudo parted -s %s unit s resizepart $(sudo parted -s %s unit s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %ss; "
                    "echo 'Resizing filesystem...'; sudo resize2fs %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, end_sector,
                    device_path, device_path, device_path
                );
            } else if (g_strcmp0(fstype, "ntfs") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition to sectors...'; sudo parted -s %s unit s resizepart $(sudo parted -s %s unit s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %ss; "
                    "echo 'Resizing filesystem...'; sudo ntfsresize -f %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, end_sector,
                    device_path, device_path, device_path
                );
            } else if (g_strcmp0(fstype, "vfat") == 0 || g_strcmp0(fstype, "fat32") == 0 || g_strcmp0(fstype, "fat") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition to sectors...'; sudo parted -s %s unit s resizepart $(sudo parted -s %s unit s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %ss; "
                    "echo 'WARNING: FAT32 resizing is risky and not always supported. You may need to reformat.'; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, end_sector,
                    device_path, device_path
                );
            } else if (g_strcmp0(fstype, "exfat") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition to sectors...'; sudo parted -s %s unit s resizepart $(sudo parted -s %s unit s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %ss; "
                    "echo 'WARNING: exFAT resizing is not natively supported everywhere. You may need to reformat.'; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, end_sector,
                    device_path, device_path
                );
            } else {
                GtkWidget *err = gtk_message_dialog_new(
                    main_window ? GTK_WINDOW(main_window) : NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Resize not implemented for this filesystem type: %s", fstype ? fstype : "Unknown"
                );
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else if (strlen(size_str) > 0) {
            if (g_strcmp0(fstype, "ext4") == 0 || g_strcmp0(fstype, "ext3") == 0 || g_strcmp0(fstype, "ext2") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition...'; sudo parted -s %s resizepart $(sudo parted -s %s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %sMiB; "
                    "echo 'Resizing filesystem...'; sudo resize2fs %s %sM; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, size_str,
                    device_path, size_str, device_path, device_path
                );
            } else if (g_strcmp0(fstype, "ntfs") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition...'; sudo parted -s %s resizepart $(sudo parted -s %s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %sMiB; "
                    "echo 'Resizing filesystem...'; sudo ntfsresize -f -s %sM %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, size_str,
                    size_str, device_path, device_path, device_path
                );
            } else if (g_strcmp0(fstype, "vfat") == 0 || g_strcmp0(fstype, "fat32") == 0 || g_strcmp0(fstype, "fat") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition...'; sudo parted -s %s resizepart $(sudo parted -s %s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %sMiB; "
                    "echo 'WARNING: FAT32 resizing is risky and not always supported. You may need to reformat.'; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, size_str,
                    device_path, device_path
                );
            } else if (g_strcmp0(fstype, "exfat") == 0) {
                cmd = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Resizing partition...'; sudo parted -s %s resizepart $(sudo parted -s %s print | awk '/^ [0-9]+/ && $NF==\"%s\" {print $1}') %sMiB; "
                    "echo 'WARNING: exFAT resizing is not natively supported everywhere. You may need to reformat.'; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Resize complete.'",
                    device_path, device_path, device_path, device_path, partition_name, size_str,
                    device_path, device_path
                );
            } else {
                GtkWidget *err = gtk_message_dialog_new(
                    main_window ? GTK_WINDOW(main_window) : NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                    "Resize not implemented for this filesystem type: %s", fstype ? fstype : "Unknown"
                );
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
        } else {
            GtkWidget *err = gtk_message_dialog_new(
                main_window ? GTK_WINDOW(main_window) : NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "You must specify either new size in MiB or both first and last sector."
            );
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        }

        if (cmd) {
            run_command(tree_view, cmd);
            g_free(cmd);
        }
    }
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

static void on_resize_dialog_destroy(GtkWidget *dialog, gpointer user_data) {
    ResizeWidgets *widgets = g_object_get_data(G_OBJECT(dialog), "widgets");
    gchar *partition_name = g_object_get_data(G_OBJECT(dialog), "partition_name");
    gchar *fstype = g_object_get_data(G_OBJECT(dialog), "fstype");
    gchar *size = g_object_get_data(G_OBJECT(dialog), "size");
    gchar *device_path = g_object_get_data(G_OBJECT(dialog), "device_path");
    if (widgets) g_free(widgets);
    if (partition_name) g_free(partition_name);
    if (fstype) g_free(fstype);
    if (size) g_free(size);
    if (device_path) g_free(device_path);
}

void on_grub_install_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_MOUNTPOINT, &mountpoint, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        gchar *disk_path = NULL;
        if (g_str_has_prefix(partition_name, "nvme") && strchr(partition_name, 'p')) {
            char *p = strrchr(device_path, 'p');
            if (p) {
                disk_path = g_strndup(device_path, p - device_path);
            }
        } else {
            char *p = device_path + strlen(device_path) - 1;
            while (p > device_path && g_ascii_isdigit(*p)) p--;
            p++;
            disk_path = g_strndup(device_path, p - device_path);
        }

        gchar *mount_dir = g_strdup("/mnt/driveassistify_grub");
        gchar *quoted_mount_dir = g_shell_quote(mount_dir);
        gchar *quoted_device_path = g_shell_quote(device_path);
        gchar *quoted_disk_path = g_shell_quote(disk_path);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This is a potentially destructive operation!\n\n"
            "If you select the wrong partition, your system may become unbootable or you may lose important data.\n\n"
            "Target partition: %s\n\n"
            "Are you sure you want to continue?",
            device_path
        );
        gint response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (response == GTK_RESPONSE_OK) {
            gchar *command = g_strdup_printf(
                "umount %s 2>/dev/null; "
                "mkdir -p %s; "
                "mount %s %s; "
                "grub-install --root-directory=%s %s; "
                "umount %s; "
                "rmdir %s; "
                "echo 'GRUB2 installation finished.'",
                quoted_device_path, quoted_mount_dir, quoted_device_path, quoted_mount_dir,
                quoted_mount_dir, quoted_disk_path, quoted_mount_dir, quoted_mount_dir
            );

            run_command(tree_view, command);

            g_free(command);
        }

        g_free(quoted_device_path);
        g_free(quoted_disk_path);
        g_free(mount_dir);
        g_free(quoted_mount_dir);
        g_free(device_path);
        g_free(disk_path);
        g_free(partition_name);
        g_free(mountpoint);
    }
}

void on_toggle_boot_flag_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        gchar *disk_path = NULL;
        gchar *part_num = NULL;
        if (g_str_has_prefix(partition_name, "nvme") && strchr(partition_name, 'p')) {
            char *p = strrchr(device_path, 'p');
            if (p) {
                disk_path = g_strndup(device_path, p - device_path);
                part_num = g_strdup(p + 1);
            }
        } else {
            char *p = device_path + strlen(device_path) - 1;
            while (p > device_path && g_ascii_isdigit(*p)) p--;
            p++;
            disk_path = g_strndup(device_path, p - device_path);
            part_num = g_strdup(p);
        }

        if (g_find_program_in_path("parted")) {
            gchar *check_cmd = g_strdup_printf(
                "parted -s %s print | grep -E '^ %s ' | grep boot",
                disk_path, part_num
            );
            int has_boot = (system(check_cmd) == 0);
            g_free(check_cmd);

            gchar *parted_cmd = NULL;
            if (has_boot) {
                parted_cmd = g_strdup_printf(
                    "parted -s %s set %s boot off && echo 'Boot flag removed.' || echo 'Failed to remove boot flag.'",
                    disk_path, part_num
                );
            } else {
                parted_cmd = g_strdup_printf(
                    "parted -s %s set %s boot on && echo 'Boot flag set.' || echo 'Failed to set boot flag.'",
                    disk_path, part_num
                );
            }

            run_command(tree_view, parted_cmd);
            g_free(parted_cmd);
        } else {
            gchar *boot_flag_hint = g_strdup_printf(
                "To manually set the boot flag for a partition using fdisk:\n\n"
                "1. Open a terminal and run:\n"
                "   sudo fdisk %s\n\n"
                "2. In fdisk, enter the following commands in order:\n"
                "   a    (press 'a' and Enter)\n"
                "   %s  (enter the partition number and press Enter)\n"
                "   w    (press 'w' and Enter)\n\n"
                "After this, the boot flag will be set. Check the result with:\n"
                "   sudo fdisk -l %s\n",
                disk_path, part_num, disk_path
            );

            show_large_text_dialog(
                GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(tree_view))),
                "Manual Boot Flag Setup",
                boot_flag_hint
            );
            g_free(boot_flag_hint);
        }

        g_free(device_path);
        g_free(disk_path);
        g_free(part_num);
        g_free(partition_name);
    }
}

void on_dd_copy_partition_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_MOUNTPOINT, &mountpoint, -1);

        GtkWidget *dialog = gtk_file_chooser_dialog_new(
            "Save Partition Image As...",
            NULL,
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Save", GTK_RESPONSE_ACCEPT,
            NULL
        );
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "partition_image.img");

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK_CANCEL,
                "WARNING! Do NOT mount this partition during copying, otherwise the image may be corrupted.\n\n"
                "Source partition: %s\nImage file: %s\n\n"
                "Are you sure you want to continue?",
                device_path, filename
            );
            gint response = gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            if (response == GTK_RESPONSE_OK) {
                gchar *quoted_device = g_shell_quote(device_path);
                gchar *quoted_file = g_shell_quote(filename);

                gchar *command = NULL;
                if (mountpoint && strlen(mountpoint) > 0 && strcmp(mountpoint, "N/A") != 0 && strcmp(mountpoint, "-") != 0) {
                    command = g_strdup_printf(
                        "umount %s 2>/dev/null; "
                        "dd if=%s of=%s bs=4M status=progress",
                        quoted_device, quoted_device, quoted_file
                    );
                } else {
                    command = g_strdup_printf(
                        "dd if=%s of=%s bs=4M status=progress",
                        quoted_device, quoted_file
                    );
                }

                run_command(tree_view, command);

                g_free(command);
                g_free(quoted_device);
                g_free(quoted_file);
            }

            g_free(device_path);
            g_free(filename);
        }
        gtk_widget_destroy(dialog);
        g_free(partition_name);
        g_free(mountpoint);
    }
}

void on_dd_restore_partition_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_MOUNTPOINT, &mountpoint, -1);

        GtkWidget *dialog = gtk_file_chooser_dialog_new(
            "Select Partition Image to Restore...",
            NULL,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Open", GTK_RESPONSE_ACCEPT,
            NULL
        );
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

            GtkWidget *confirm = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK_CANCEL,
                "WARNING: This is a potentially destructive operation!\n\n"
                "If you select the wrong partition, your system may become unbootable or you may lose important data.\n\n"
                "Target partition: %s\nImage file: %s\n\n"
                "Are you sure you want to continue?",
                device_path, filename
            );
            gint response = gtk_dialog_run(GTK_DIALOG(confirm));
            gtk_widget_destroy(confirm);

            if (response == GTK_RESPONSE_OK) {
                gchar *quoted_device = g_shell_quote(device_path);
                gchar *quoted_file = g_shell_quote(filename);

                gchar *command = NULL;
                if (mountpoint && strlen(mountpoint) > 0 && strcmp(mountpoint, "N/A") != 0 && strcmp(mountpoint, "-") != 0) {
                    command = g_strdup_printf(
                        "echo 'WARNING! Do NOT mount this partition during restore, otherwise the data may be corrupted.'; "
                        "umount %s 2>/dev/null; "
                        "dd if=%s of=%s bs=4M status=progress",
                        quoted_device, quoted_file, quoted_device
                    );
                } else {
                    command = g_strdup_printf(
                        "echo 'WARNING! Do NOT mount this partition during restore, otherwise the data may be corrupted.'; "
                        "dd if=%s of=%s bs=4M status=progress",
                        quoted_file, quoted_device
                    );
                }

                run_command(tree_view, command);

                g_free(command);
                g_free(quoted_device);
                g_free(quoted_file);
            }

            g_free(device_path);
            g_free(filename);
        }
        gtk_widget_destroy(dialog);
        g_free(partition_name);
        g_free(mountpoint);
    }
}

void on_delete_partition_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_MOUNTPOINT, &mountpoint, -1);
        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        gchar disk[64], part_num[16];
        int disk_len = 0;
        for (int i = strlen(partition_name) - 1; i >= 0; --i) {
            if (!isdigit(partition_name[i])) {
                disk_len = i + 1;
                break;
            }
        }
        strncpy(disk, partition_name, disk_len);
        disk[disk_len] = '\0';
        strcpy(part_num, partition_name + disk_len);

        gchar *disk_path = g_strdup_printf("/dev/%s", disk);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This will irreversibly delete partition %s from %s using fdisk.\n\n"
            "All data and the filesystem in this partition will be lost and the area will become Free Space.\n\n"
            "Are you sure you want to continue?",
            partition_name, disk_path
        );
        gint warn_response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (warn_response == GTK_RESPONSE_OK) {
            gchar *command = NULL;
            if (mountpoint && strlen(mountpoint) > 0 && strcmp(mountpoint, "N/A") != 0 && strcmp(mountpoint, "-") != 0) {
                command = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Deleting partition %s...'; "
                    "echo -e 'd\\n%s\\nw\\n' | sudo fdisk %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Partition deleted and partition table re-read.'",
                    device_path, device_path, partition_name, part_num, disk_path, disk_path, disk_path
                );
            } else {
                command = g_strdup_printf(
                    "echo 'Deleting partition %s...'; "
                    "echo -e 'd\\n%s\\nw\\n' | sudo fdisk %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Partition deleted and partition table re-read.'",
                    partition_name, part_num, disk_path, disk_path, disk_path
                );
            }
            run_command(tree_view, command);
            g_free(command);
        }
        g_free(device_path);
        g_free(disk_path);
        g_free(partition_name);
        if (mountpoint) g_free(mountpoint);
    }
}

void on_shred_fs_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_MOUNTPOINT, &mountpoint, -1);
        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This will irreversibly destroy the filesystem and all data on %s using multiple overwrite passes (shred).\n\n"
            "Are you sure you want to continue?",
            device_path
        );
        gint warn_response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (warn_response == GTK_RESPONSE_OK) {
            gchar *command = NULL;
            if (mountpoint && strlen(mountpoint) > 0 && strcmp(mountpoint, "N/A") != 0 && strcmp(mountpoint, "-") != 0) {
                command = g_strdup_printf(
                    "echo 'Unmounting %s...'; umount %s 2>/dev/null; "
                    "echo 'Shredding %s...'; shred -v -n 3 -z %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Filesystem destroyed and partition table re-read.'",
                    device_path, device_path, device_path, device_path, device_path, device_path
                );
            } else {
                command = g_strdup_printf(
                    "echo 'Shredding %s...'; shred -v -n 3 -z %s; "
                    "(sudo partprobe %s || sudo blockdev --rereadpt %s); "
                    "echo 'Filesystem destroyed and partition table re-read.'",
                    device_path, device_path, device_path, device_path
                );
            }
            run_command(tree_view, command);
            g_free(command);
        }
        g_free(device_path);
        g_free(partition_name);
        if (mountpoint) g_free(mountpoint);
    }
}

void on_dd_erase_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This operation will irreversibly erase all data on the selected partition or disk!\n\n"
            "If you select the wrong device, you may lose your operating system or important files.\n\n"
            "Target: %s\n\n"
            "Are you sure you want to continue?",
            device_path
        );
        gint response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (response == GTK_RESPONSE_OK) {
            gchar *quoted_device = g_shell_quote(device_path);
            gchar *command = g_strdup_printf(
                "dd if=/dev/urandom of=%s bs=1M status=progress && echo 'Disk erased successfully'",
                quoted_device
            );
            run_command(tree_view, command);
            g_free(command);
            g_free(quoted_device);
        }

        g_free(device_path);
        g_free(partition_name);
    }
}

void on_dd_multiple_erase_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This operation will irreversibly erase all data on the selected partition or disk using multiple overwrite passes!\n\n"
            "If you select the wrong device, you may lose your operating system or important files.\n\n"
            "Target: %s\n\n"
            "Are you sure you want to continue?",
            device_path
        );
        gint response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (response == GTK_RESPONSE_OK) {
            gchar *quoted_device = g_shell_quote(device_path);
            gchar *command = g_strdup_printf(
                "dd if=/dev/urandom of=%s bs=1M status=progress; "
                "dd if=/dev/zero of=%s bs=1M status=progress; "
                "dd if=/dev/zero of=%s bs=1M status=progress; "
                "dd if=/dev/full of=%s bs=1M status=progress && echo 'Disk erased successfully'",
                quoted_device, quoted_device, quoted_device, quoted_device
            );
            run_command(tree_view, command);
            g_free(command);
            g_free(quoted_device);
        }

        g_free(device_path);
        g_free(partition_name);
    }
}

void clean_string(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (isalnum(*src) || *src == '/') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

void get_value(const char *src, const char *key, char *buf, size_t buflen) {
    char *p = strstr(src, key);
    if (!p) { buf[0] = 0; return; }
    p = strchr(p, '\"');
    if (!p) { buf[0] = 0; return; }
    ++p;
    char *end = strchr(p, '\"');
    if (!end) { buf[0] = 0; return; }
    size_t len = end - p < buflen-1 ? end - p : buflen-1;
    strncpy(buf, p, len);
    buf[len] = 0;
}

typedef struct {
    char name[100], size[100], type[100], fstype[100], mountpoint[100], uuid[100], model[100];
    GdkRGBA color;
    int has_color;
    char font_color[16];
    int weight;
} DiskRow;

int compare_rows(const void *a, const void *b) {
    const DiskRow *ra = (const DiskRow*)a;
    const DiskRow *rb = (const DiskRow*)b;

    char root_a[100], root_b[100];
    int i = 0;
    while (ra->name[i] && (ra->name[i] < '0' || ra->name[i] > '9')) {
        root_a[i] = ra->name[i];
        i++;
    }
    root_a[i] = 0;
    i = 0;
    while (rb->name[i] && (rb->name[i] < '0' || rb->name[i] > '9')) {
        root_b[i] = rb->name[i];
        i++;
    }
    root_b[i] = 0;

    int cmp = strcmp(root_a, root_b);
    if (cmp != 0)
        return cmp;

    int is_disk_a = strcmp(ra->type, "disk") == 0;
    int is_disk_b = strcmp(rb->type, "disk") == 0;
    if (is_disk_a && !is_disk_b) return -1;
    if (!is_disk_a && is_disk_b) return 1;

    return strcmp(ra->name, rb->name);
}

void show_disk_list(GtkWidget *widget, gpointer tree_view) {
    GtkListStore *store;
    GtkTreeIter iter;
    FILE *fp;
    char path[1035];

    DiskRow rows[256];
    int row_count = 0;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_view)));
    gtk_list_store_clear(store);

    fp = popen("lsblk -P -o NAME,SIZE,TYPE,FSTYPE,MOUNTPOINT,UUID,MODEL", "r");
    if (fp == NULL) {
        g_print("Failed to run command\n");
        exit(1);
    }

    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        char name[100], size[100], type[100], fstype[100], mountpoint[100], uuid[100], model[100];
        get_value(path, "NAME=", name, sizeof(name));
        get_value(path, "SIZE=", size, sizeof(size));
        get_value(path, "TYPE=", type, sizeof(type));
        get_value(path, "FSTYPE=", fstype, sizeof(fstype));
        get_value(path, "MOUNTPOINT=", mountpoint, sizeof(mountpoint));
        get_value(path, "UUID=", uuid, sizeof(uuid));
        get_value(path, "MODEL=", model, sizeof(model));

        clean_string(name);
        clean_string(fstype);
        clean_string(mountpoint);
        clean_string(uuid);
        clean_string(model);

        if (strcmp(type, "disk") == 0) {
            if (strlen(mountpoint) == 0 || strcmp(mountpoint, "-") == 0) {
                strcpy(mountpoint, "N/A");
            }
            if (strlen(uuid) == 0 || strcmp(uuid, "-") == 0) {
                strcpy(uuid, "N/A");
            }
            if (strlen(model) == 0 || strcmp(model, "-") == 0) {
                strcpy(model, "N/A");
            }
        }

        strcpy(rows[row_count].name, name);
        strcpy(rows[row_count].size, size);
        strcpy(rows[row_count].type, type);
        strcpy(rows[row_count].fstype, fstype);
        strcpy(rows[row_count].mountpoint, mountpoint);
        strcpy(rows[row_count].uuid, uuid);
        strcpy(rows[row_count].model, model);

        if (strcmp(type, "disk") == 0) {
            gdk_rgba_parse(&rows[row_count].color, "#e6f1fa");
            rows[row_count].has_color = 1;
            strcpy(rows[row_count].font_color, "#0057ae");
            rows[row_count].weight = 700;
        } else {
            rows[row_count].has_color = 0;
            rows[row_count].font_color[0] = 0;
            rows[row_count].weight = 400;
        }
        row_count++;
    }

    pclose(fp);

    qsort(rows, row_count, sizeof(DiskRow), compare_rows);

    for (int i = 0; i < row_count; ++i) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COL_NAME, rows[i].name,
            COL_SIZE, rows[i].size,
            COL_TYPE, rows[i].type,
            COL_FSTYPE, rows[i].fstype,
            COL_MOUNTPOINT, rows[i].mountpoint,
            COL_UUID, rows[i].uuid,
            COL_MODEL, rows[i].model,
            COL_ROW_COLOR, rows[i].has_color ? &rows[i].color : NULL,
            COL_FONT_COLOR, rows[i].font_color[0] ? rows[i].font_color : NULL,
            COL_WEIGHT, rows[i].weight,
            -1);
    }
}

void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    GtkWidget *menu = gtk_menu_new();

    GtkWidget *info_menu = gtk_menu_new();
    GtkWidget *info_root = gtk_menu_item_new_with_label("Information");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(info_root), info_menu);

    GtkWidget *info_item = gtk_menu_item_new_with_label("Device Information (lsblk, fdisk, parted, blkid)");
    g_signal_connect(info_item, "activate", G_CALLBACK(on_device_info_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), info_item);

    GtkWidget *smartctl_item = gtk_menu_item_new_with_label("S.M.A.R.T. Information (smartctl)");
    g_signal_connect(smartctl_item, "activate", G_CALLBACK(on_smartctl_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), smartctl_item);

    GtkWidget *show_areas_item = gtk_menu_item_new_with_label("Show Filesystems and Free Space (parted)");
    g_signal_connect(show_areas_item, "activate", G_CALLBACK(on_show_disk_areas_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), show_areas_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), info_root);

    GtkWidget *scan_menu = gtk_menu_new();
    GtkWidget *scan_root = gtk_menu_item_new_with_label("Scan & Repair");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(scan_root), scan_menu);

    GtkWidget *e2fsck_item = gtk_menu_item_new_with_label("Check and Repair EXT Filesystem (e2fsck) => POTENTIALLY DANGEROUS! May cause data loss if errors are present.");
    g_signal_connect(e2fsck_item, "activate", G_CALLBACK(on_e2fsck_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), e2fsck_item);

    GtkWidget *menuitem_ext_repair_deep = gtk_menu_item_new_with_label("Deep EXT Recovery (e2fsck) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(menuitem_ext_repair_deep, "activate", G_CALLBACK(on_ext_repair_deep_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), menuitem_ext_repair_deep);

    GtkWidget *ntfsfix_item = gtk_menu_item_new_with_label("Repair NTFS Filesystem (ntfsfix) => POTENTIALLY DANGEROUS! May cause data loss.");
    g_signal_connect(ntfsfix_item, "activate", G_CALLBACK(on_ntfsfix_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), ntfsfix_item);

    GtkWidget *ntfsresize_item = gtk_menu_item_new_with_label("NTFS Resize Info (ntfsresize)");
    g_signal_connect(ntfsresize_item, "activate", G_CALLBACK(on_ntfsresize_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), ntfsresize_item);

    GtkWidget *diskscan_item = gtk_menu_item_new_with_label("Disk Surface Scan (diskscan)");
    g_signal_connect(diskscan_item, "activate", G_CALLBACK(on_diskscan_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), diskscan_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), scan_root);

    GtkWidget *mount_menu = gtk_menu_new();
    GtkWidget *mount_root = gtk_menu_item_new_with_label("Mount & Unmount");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(mount_root), mount_menu);

    GtkWidget *mount_item = gtk_menu_item_new_with_label("Mount Partition (mount)");
    g_signal_connect(mount_item, "activate", G_CALLBACK(on_mount_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(mount_menu), mount_item);

    GtkWidget *umount_item = gtk_menu_item_new_with_label("Unmount Partition (umount)");
    g_signal_connect(umount_item, "activate", G_CALLBACK(on_umount_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(mount_menu), umount_item);

    GtkWidget *umount_l_item = gtk_menu_item_new_with_label("Lazy Unmount (umount -l)");
    g_signal_connect(umount_l_item, "activate", G_CALLBACK(on_umount_l_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(mount_menu), umount_l_item);

    GtkWidget *umount_f_item = gtk_menu_item_new_with_label("Forced Unmount (umount -f)");
    g_signal_connect(umount_f_item, "activate", G_CALLBACK(on_umount_f_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(mount_menu), umount_f_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), mount_root);

    GtkWidget *fs_menu = gtk_menu_new();
    GtkWidget *fs_root = gtk_menu_item_new_with_label("Filesystem & Partition Tools");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fs_root), fs_menu);

    GtkWidget *rename_item = gtk_menu_item_new_with_label("Rename Partition (label utility)");
    g_signal_connect(rename_item, "activate", G_CALLBACK(on_rename_partition_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), rename_item);

    GtkWidget *partition_table_item = gtk_menu_item_new_with_label("Create Partition Table (parted/fdisk) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(partition_table_item, "activate", G_CALLBACK(on_partition_table_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), partition_table_item);

    GtkWidget *mkfs_item = gtk_menu_item_new_with_label("Create Filesystem (mkfs) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(mkfs_item, "activate", G_CALLBACK(on_mkfs_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), mkfs_item);

    GtkWidget *resize_item = gtk_menu_item_new_with_label("Resize/Move Partition (ext2/3/4, ntfsresize, fatresize, exfatprogs) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(resize_item, "activate", G_CALLBACK(on_resize_partition_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), resize_item);

    GtkWidget *grub_install_item = gtk_menu_item_new_with_label("Install GRUB2 Bootloader (grub-install) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(grub_install_item, "activate", G_CALLBACK(on_grub_install_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), grub_install_item);

    GtkWidget *toggle_boot_flag_item = gtk_menu_item_new_with_label("Toggle Boot Flag (parted/fdisk) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(toggle_boot_flag_item, "activate", G_CALLBACK(on_toggle_boot_flag_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), toggle_boot_flag_item);

    GtkWidget *dd_copy_partition_item = gtk_menu_item_new_with_label("Create Partition Image (dd)");
    g_signal_connect(dd_copy_partition_item, "activate", G_CALLBACK(on_dd_copy_partition_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), dd_copy_partition_item);

    GtkWidget *dd_restore_partition_item = gtk_menu_item_new_with_label("Restore Partition from Image (dd restore) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(dd_restore_partition_item, "activate", G_CALLBACK(on_dd_restore_partition_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), dd_restore_partition_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), fs_root);

    GtkWidget *delete_menu = gtk_menu_new();
    GtkWidget *delete_root = gtk_menu_item_new_with_label("Delete Filesystems & Data");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(delete_root), delete_menu);

    GtkWidget *delete_item = gtk_menu_item_new_with_label("Delete Partition (fdisk) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(delete_item, "activate", G_CALLBACK(on_delete_partition_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(delete_menu), delete_item);

    GtkWidget *shred_item = gtk_menu_item_new_with_label("Destroy Filesystem (shred) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(shred_item, "activate", G_CALLBACK(on_shred_fs_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(delete_menu), shred_item);
    
    GtkWidget *dd_erase_item = gtk_menu_item_new_with_label("Erase All Data on Selected Disk/Partition (dd) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(dd_erase_item, "activate", G_CALLBACK(on_dd_erase_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(delete_menu), dd_erase_item);

    GtkWidget *dd_multiple_erase_item = gtk_menu_item_new_with_label("Erase All Data on Selected Disk/Partition with Multiple Overwrites (dd) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(dd_multiple_erase_item, "activate", G_CALLBACK(on_dd_multiple_erase_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(delete_menu), dd_multiple_erase_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delete_root);

    gtk_widget_show_all(menu);

    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

void create_termsofuse_window(GtkWidget *parent) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Terms of Use");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 600);
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_window), GTK_SHADOW_NONE);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);

    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view), 5);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    const gchar *termsofuse_text = 
"TERMS OF USE\n\n"
"1. Usage:\n"
"You are granted a non-exclusive, non-transferable license to use the Program under the terms of the GNU General Public License (GPL) Version 3.0. The term \"Program\" refers to the software package or product distributed under this License. You may use, copy, modify, and distribute the Program freely, provided that all copies and derivative works are licensed under the GPL and include this license notice.\n\n"
"2. License:\n"
"This Program is licensed under the GNU General Public License (GPL) Version 3.0, which ensures that users have the freedom to run, study, share, and modify the software. A copy of the GPL license is included with the Program package, or you can access it at https://www.gnu.org/licenses/gpl-3.0.html.\n\n"
"3. Source Code Availability:\n"
"As required by the GNU General Public License (GPL), the full source code of this Program is available and can be obtained from the official repository or package distribution. If you did not receive a copy of the source code, you may request it from the developer. Additionally, you have the right to access and modify the source code under the terms of this License.\n\n"
"4. Disclaimer of Warranties:\n"
"The Program is provided \"as is,\" without any warranties, express or implied, including but not limited to the implied warranties of merchantability or fitness for a particular purpose. The developers make no representations or warranties regarding the use or performance of the Program.\n\n"
"5. Limitation of Liability:\n"
"In no event shall the developers be liable for any direct, indirect, incidental, special, exemplary, or consequential damages, including but not limited to damages for loss of data or profit, arising out of or in connection with the use of or inability to use the Program, even if advised of the possibility of such damages.\n\n"
"6. Modifications to the Program:\n"
"You may modify and distribute modified versions of the Program, provided you comply with the terms of the GNU General Public License (GPL). The developers reserve the right to modify, update, or discontinue the Program at their discretion.\n\n"
"7. Compliance with Laws:\n"
"You are responsible for complying with all applicable local, state, national, and international laws in connection with your use of the Program.\n\n"
"8. Copyright:\n"
"Copyright (C) 2024–2025 Maksym Nazar.\n"
"Created with the assistance of ChatGPT.\n"
"This work is licensed under the GNU General Public License (GPL) Version 3.0.\n\n"
"9. Contact:\n"
"For inquiries, please contact us at:\n"
"Email: maximkursua@gmail.com\n";

    gtk_text_buffer_set_text(buffer, termsofuse_text, -1);

    gtk_widget_show_all(window);
}

void create_license_window(GtkWidget *parent) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "License");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 600);
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);

    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(text_view), 20);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(text_view), 5);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(text_view), 5);

    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    const gchar *license_text = "GNU GENERAL PUBLIC LICENSE\n\n"
"Version 3, 29 June 2007\n\n"
"Copyright © 2007 Free Software Foundation, Inc. <https://fsf.org/>\n\n"
"Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed.\n\n"
"Preamble\n\n"
"The GNU General Public License is a free, copyleft license for software and other kinds of works.\n\n"
"The licenses for most software and other practical works are designed to take away your freedom to share and change the works. By contrast, the GNU General Public License is intended to guarantee your freedom to share and change all versions of a program--to make sure it remains free software for all its users. We, the Free Software Foundation, use the GNU General Public License for most of our software; it applies also to any other work released this way by its authors. You can apply it to your programs, too.\n\n"
"When we speak of free software, we are referring to freedom, not price. Our General Public Licenses are designed to make sure that you have the freedom to distribute copies of free software (and charge for them if you wish), that you receive source code or can get it if you want it, that you can change the software or use pieces of it in new free programs, and that you know you can do these things.\n\n"
"To protect your rights, we need to prevent others from denying you these rights or asking you to surrender the rights. Therefore, you have certain responsibilities if you distribute copies of the software, or if you modify it: responsibilities to respect the freedom of others.\n\n"
"For example, if you distribute copies of such a program, whether gratis or for a fee, you must pass on to the recipients the same freedoms that you received. You must make sure that they, too, receive or can get the source code. And you must show them these terms so they know their rights.\n\n"
"Developers that use the GNU GPL protect your rights with two steps: (1) assert copyright on the software, and (2) offer you this License giving you legal permission to copy, distribute and/or modify it.\n\n"
"For the developers' and authors' protection, the GPL clearly explains that there is no warranty for this free software. For both users' and authors' sake, the GPL requires that modified versions be marked as changed, so that their problems will not be attributed erroneously to authors of previous versions.\n\n"
"Some devices are designed to deny users access to install or run modified versions of the software inside them, although the manufacturer can do so. This is fundamentally incompatible with the aim of protecting users' freedom to change the software. The systematic pattern of such abuse occurs in the area of products for individuals to use, which is precisely where it is most unacceptable. Therefore, we have designed this version of the GPL to prohibit the practice for those products. If such problems arise substantially in other domains, we stand ready to extend this provision to those domains in future versions of the GPL, as needed to protect the freedom of users.\n\n"
"Finally, every program is threatened constantly by software patents. States should not allow patents to restrict development and use of software on general-purpose computers, but in those that do, we wish to avoid the special danger that patents applied to a free program could make it effectively proprietary. To prevent this, the GPL assures that patents cannot be used to render the program non-free.\n\n"
"The precise terms and conditions for copying, distribution and modification follow.\n\n"
"TERMS AND CONDITIONS\n\n"
"0. Definitions.\n\n"
"This License” refers to version 3 of the GNU General Public License.\n\n"
"“Copyright” also means copyright-like laws that apply to other kinds of works, such as semiconductor masks.\n\n"
"“The Program” refers to any copyrightable work licensed under this License. Each licensee is addressed as “you”. “Licensees” and “recipients” may be individuals or organizations.\n\n"
"To “modify” a work means to copy from or adapt all or part of the work in a fashion requiring copyright permission, other than the making of an exact copy. The resulting work is called a “modified version” of the earlier work or a work “based on” the earlier work.\n\n"
"A “covered work” means either the unmodified Program or a work based on the Program.\n\n"
"To “propagate” a work means to do anything with it that, without permission, would make you directly or secondarily liable for infringement under applicable copyright law, except executing it on a computer or modifying a private copy. Propagation includes copying, distribution (with or without modification), making available to the public, and in some countries other activities as well.\n\n"
"To “convey” a work means any kind of propagation that enables other parties to make or receive copies. Mere interaction with a user through a computer network, with no transfer of a copy, is not conveying.\n\n"
"An interactive user interface displays “Appropriate Legal Notices” to the extent that it includes a convenient and prominently visible feature that (1) displays an appropriate copyright notice, and (2) tells the user that there is no warranty for the work (except to the extent that warranties are provided), that licensees may convey the work under this License, and how to view a copy of this License. If the interface presents a list of user commands or options, such as a menu, a prominent item in the list meets this criterion.\n\n"
"1. Source Code.\n\n"
"The “source code” for a work means the preferred form of the work for making modifications to it. “Object code” means any non-source form of a work.\n\n"
"A “Standard Interface” means an interface that either is an official standard defined by a recognized standards body, or, in the case of interfaces specified for a particular programming language, one that is widely used among developers working in that language.\n\n"
"The “System Libraries” of an executable work include anything, other than the work as a whole, that (a) is included in the normal form of packaging a Major Component, but which is not part of that Major Component, and (b) serves only to enable use of the work with that Major Component, or to implement a Standard Interface for which an implementation is available to the public in source code form. A “Major Component”, in this context, means a major essential component (kernel, window system, and so on) of the specific operating system (if any) on which the executable work runs, or a compiler used to produce the work, or an object code interpreter used to run it.\n\n"
"The “Corresponding Source” for a work in object code form means all the source code needed to generate, install, and (for an executable work) run the object code and to modify the work, including scripts to control those activities. However, it does not include the work's System Libraries, or general-purpose tools or generally available free programs which are used unmodified in performing those activities but which are not part of the work. For example, Corresponding Source includes interface definition files associated with source files for the work, and the source code for shared libraries and dynamically linked subprograms that the work is specifically designed to require, such as by intimate data communication or control flow between those subprograms and other parts of the work.\n\n"
"The Corresponding Source need not include anything that users can regenerate automatically from other parts of the Corresponding Source.\n\n"
"The Corresponding Source for a work in source code form is that same work.\n\n"
"2. Basic Permissions.\n\n"
"All rights granted under this License are granted for the term of copyright on the Program, and are irrevocable provided the stated conditions are met. This License explicitly affirms your unlimited permission to run the unmodified Program. The output from running a covered work is covered by this License only if the output, given its content, constitutes a covered work. This License acknowledges your rights of fair use or other equivalent, as provided by copyright law.\n\n"
"You may make, run and propagate covered works that you do not convey, without conditions so long as your license otherwise remains in force. You may convey covered works to others for the sole purpose of having them make modifications exclusively for you, or provide you with facilities for running those works, provided that you comply with the terms of this License in conveying all material for which you do not control copyright. Those thus making or running the covered works for you must do so exclusively on your behalf, under your direction and control, on terms that prohibit them from making any copies of your copyrighted material outside their relationship with you.\n\n"
"Conveying under any other circumstances is permitted solely under the conditions stated below. Sublicensing is not allowed; section 10 makes it unnecessary.\n\n"
"3. Protecting Users' Legal Rights From Anti-Circumvention Law.\n\n"
"No covered work shall be deemed part of an effective technological measure under any applicable law fulfilling obligations under article 11 of the WIPO copyright treaty adopted on 20 December 1996, or similar laws prohibiting or restricting circumvention of such measures.\n\n"
"When you convey a covered work, you waive any legal power to forbid circumvention of technological measures to the extent such circumvention is effected by exercising rights under this License with respect to the covered work, and you disclaim any intention to limit operation or modification of the work as a means of enforcing, against the work's users, your or third parties' legal rights to forbid circumvention of technological measures.\n\n"
"4. Conveying Verbatim Copies.\n\n"
"You may convey verbatim copies of the Program's source code as you receive it, in any medium, provided that you conspicuously and appropriately publish on each copy an appropriate copyright notice; keep intact all notices stating that this License and any non-permissive terms added in accord with section 7 apply to the code; keep intact all notices of the absence of any warranty; and give all recipients a copy of this License along with the Program.\n\n"
"You may charge any price or no price for each copy that you convey, and you may offer support or warranty protection for a fee.\n\n"
"5. Conveying Modified Source Versions.\n\n"
"You may convey a work based on the Program, or the modifications to produce it from the Program, in the form of source code under the terms of section 4, provided that you also meet all of these conditions:\n\n"
"    a) The work must carry prominent notices stating that you modified it, and giving a relevant date.\n\n"
"    b) The work must carry prominent notices stating that it is released under this License and any conditions added under section 7. This requirement modifies the requirement in section 4 to “keep intact all notices”.\n\n"
"    c) You must license the entire work, as a whole, under this License to anyone who comes into possession of a copy. This License will therefore apply, along with any applicable section 7 additional terms, to the whole of the work, and all its parts, regardless of how they are packaged. This License gives no permission to license the work in any other way, but it does not invalidate such permission if you have separately received it.\n\n"
"    d) If the work has interactive user interfaces, each must display Appropriate Legal Notices; however, if the Program has interactive interfaces that do not display Appropriate Legal Notices, your work need not make them do so.\n\n"
"A compilation of a covered work with other separate and independent works, which are not by their nature extensions of the covered work, and which are not combined with it such as to form a larger program, in or on a volume of a storage or distribution medium, is called an “aggregate” if the compilation and its resulting copyright are not used to limit the access or legal rights of the compilation's users beyond what the individual works permit. Inclusion of a covered work in an aggregate does not cause this License to apply to the other parts of the aggregate.\n\n"
"6. Conveying Non-Source Forms.\n\n"
"You may convey a covered work in object code form under the terms of sections 4 and 5, provided that you also convey the machine-readable Corresponding Source under the terms of this License, in one of these ways:\n\n"
"    a) Convey the object code in, or embodied in, a physical product (including a physical distribution medium), accompanied by the Corresponding Source fixed on a durable physical medium customarily used for software interchange.\n\n"
"    b) Convey the object code in, or embodied in, a physical product (including a physical distribution medium), accompanied by a written offer, valid for at least three years and valid for as long as you offer spare parts or customer support for that product model, to give anyone who possesses the object code either (1) a copy of the Corresponding Source for all the software in the product that is covered by this License, on a durable physical medium customarily used for software interchange, for a price no more than your reasonable cost of physically performing this conveying of source, or (2) access to copy the Corresponding Source from a network server at no charge.\n\n"
"    c) Convey individual copies of the object code with a copy of the written offer to provide the Corresponding Source. This alternative is allowed only occasionally and noncommercially, and only if you received the object code with such an offer, in accord with subsection 6b.\n\n"
"    d) Convey the object code by offering access from a designated place (gratis or for a charge), and offer equivalent access to the Corresponding Source in the same way through the same place at no further charge. You need not require recipients to copy the Corresponding Source along with the object code. If the place to copy the object code is a network server, the Corresponding Source may be on a different server (operated by you or a third party) that supports equivalent copying facilities, provided you maintain clear directions next to the object code saying where to find the Corresponding Source. Regardless of what server hosts the Corresponding Source, you remain obligated to ensure that it is available for as long as needed to satisfy these requirements.\n\n"
"    e) Convey the object code using peer-to-peer transmission, provided you inform other peers where the object code and Corresponding Source of the work are being offered to the general public at no charge under subsection 6d.\n\n"
"A separable portion of the object code, whose source code is excluded from the Corresponding Source as a System Library, need not be included in conveying the object code work.\n\n"
"A “User Product” is either (1) a “consumer product”, which means any tangible personal property which is normally used for personal, family, or household purposes, or (2) anything designed or sold for incorporation into a dwelling. In determining whether a product is a consumer product, doubtful cases shall be resolved in favor of coverage. For a particular product received by a particular user, “normally used” refers to a typical or common use of that class of product, regardless of the status of the particular user or of the way in which the particular user actually uses, or expects or is expected to use, the product. A product is a consumer product regardless of whether the product has substantial commercial, industrial or non-consumer uses, unless such uses represent the only significant mode of use of the product.\n\n"
"“Installation Information” for a User Product means any methods, procedures, authorization keys, or other information required to install and execute modified versions of a covered work in that User Product from a modified version of its Corresponding Source. The information must suffice to ensure that the continued functioning of the modified object code is in no case prevented or interfered with solely because modification has been made.\n\n"
"If you convey an object code work under this section in, or with, or specifically for use in, a User Product, and the conveying occurs as part of a transaction in which the right of possession and use of the User Product is transferred to the recipient in perpetuity or for a fixed term (regardless of how the transaction is characterized), the Corresponding Source conveyed under this section must be accompanied by the Installation Information. But this requirement does not apply if neither you nor any third party retains the ability to install modified object code on the User Product (for example, the work has been installed in ROM).\n\n"
"The requirement to provide Installation Information does not include a requirement to continue to provide support service, warranty, or updates for a work that has been modified or installed by the recipient, or for the User Product in which it has been modified or installed. Access to a network may be denied when the modification itself materially and adversely affects the operation of the network or violates the rules and protocols for communication across the network.\n\n"
"Corresponding Source conveyed, and Installation Information provided, in accord with this section must be in a format that is publicly documented (and with an implementation available to the public in source code form), and must require no special password or key for unpacking, reading or copying.\n\n"
"7. Additional Terms.\n\n"
"“Additional permissions” are terms that supplement the terms of this License by making exceptions from one or more of its conditions. Additional permissions that are applicable to the entire Program shall be treated as though they were included in this License, to the extent that they are valid under applicable law. If additional permissions apply only to part of the Program, that part may be used separately under those permissions, but the entire Program remains governed by this License without regard to the additional permissions.\n\n"
"When you convey a copy of a covered work, you may at your option remove any additional permissions from that copy, or from any part of it. (Additional permissions may be written to require their own removal in certain cases when you modify the work.) You may place additional permissions on material, added by you to a covered work, for which you have or can give appropriate copyright permission.\n\n"
"Notwithstanding any other provision of this License, for material you add to a covered work, you may (if authorized by the copyright holders of that material) supplement the terms of this License with terms:\n\n"
"    a) Disclaiming warranty or limiting liability differently from the terms of sections 15 and 16 of this License; or\n\n"
"    b) Requiring preservation of specified reasonable legal notices or author attributions in that material or in the Appropriate Legal Notices displayed by works containing it; or\n\n"
"    c) Prohibiting misrepresentation of the origin of that material, or requiring that modified versions of such material be marked in reasonable ways as different from the original version; or\n\n"
"    d) Limiting the use for publicity purposes of names of licensors or authors of the material; or\n\n"
"    e) Declining to grant rights under trademark law for use of some trade names, trademarks, or service marks; or\n\n"
"    f) Requiring indemnification of licensors and authors of that material by anyone who conveys the material (or modified versions of it) with contractual assumptions of liability to the recipient, for any liability that these contractual assumptions directly impose on those licensors and authors.\n\n"
"All other non-permissive additional terms are considered “further restrictions” within the meaning of section 10. If the Program as you received it, or any part of it, contains a notice stating that it is governed by this License along with a term that is a further restriction, you may remove that term. If a license document contains a further restriction but permits relicensing or conveying under this License, you may add to a covered work material governed by the terms of that license document, provided that the further restriction does not survive such relicensing or conveying.\n\n"
"If you add terms to a covered work in accord with this section, you must place, in the relevant source files, a statement of the additional terms that apply to those files, or a notice indicating where to find the applicable terms.\n\n"
"Additional terms, permissive or non-permissive, may be stated in the form of a separately written license, or stated as exceptions; the above requirements apply either way.\n\n"
"8. Termination.\n\n"
"You may not propagate or modify a covered work except as expressly provided under this License. Any attempt otherwise to propagate or modify it is void, and will automatically terminate your rights under this License (including any patent licenses granted under the third paragraph of section 11).\n\n"
"However, if you cease all violation of this License, then your license from a particular copyright holder is reinstated (a) provisionally, unless and until the copyright holder explicitly and finally terminates your license, and (b) permanently, if the copyright holder fails to notify you of the violation by some reasonable means prior to 60 days after the cessation.\n\n"
"Moreover, your license from a particular copyright holder is reinstated permanently if the copyright holder notifies you of the violation by some reasonable means, this is the first time you have received notice of violation of this License (for any work) from that copyright holder, and you cure the violation prior to 30 days after your receipt of the notice.\n\n"
"Termination of your rights under this section does not terminate the licenses of parties who have received copies or rights from you under this License. If your rights have been terminated and not permanently reinstated, you do not qualify to receive new licenses for the same material under section 10.\n\n"
"9. Acceptance Not Required for Having Copies.\n\n"
"You are not required to accept this License in order to receive or run a copy of the Program. Ancillary propagation of a covered work occurring solely as a consequence of using peer-to-peer transmission to receive a copy likewise does not require acceptance. However, nothing other than this License grants you permission to propagate or modify any covered work. These actions infringe copyright if you do not accept this License. Therefore, by modifying or propagating a covered work, you indicate your acceptance of this License to do so.\n\n"
"10. Automatic Licensing of Downstream Recipients.\n\n"
"Each time you convey a covered work, the recipient automatically receives a license from the original licensors, to run, modify and propagate that work, subject to this License. You are not responsible for enforcing compliance by third parties with this License.\n\n"
"An “entity transaction” is a transaction transferring control of an organization, or substantially all assets of one, or subdividing an organization, or merging organizations. If propagation of a covered work results from an entity transaction, each party to that transaction who receives a copy of the work also receives whatever licenses to the work the party's predecessor in interest had or could give under the previous paragraph, plus a right to possession of the Corresponding Source of the work from the predecessor in interest, if the predecessor has it or can get it with reasonable efforts.\n\n"
"You may not impose any further restrictions on the exercise of the rights granted or affirmed under this License. For example, you may not impose a license fee, royalty, or other charge for exercise of rights granted under this License, and you may not initiate litigation (including a cross-claim or counterclaim in a lawsuit) alleging that any patent claim is infringed by making, using, selling, offering for sale, or importing the Program or any portion of it.\n\n"
"11. Patents.\n\n"
"A “contributor” is a copyright holder who authorizes use under this License of the Program or a work on which the Program is based. The work thus licensed is called the contributor's “contributor version”.\n\n"
"A contributor's “essential patent claims” are all patent claims owned or controlled by the contributor, whether already acquired or hereafter acquired, that would be infringed by some manner, permitted by this License, of making, using, or selling its contributor version, but do not include claims that would be infringed only as a consequence of further modification of the contributor version. For purposes of this definition, “control” includes the right to grant patent sublicenses in a manner consistent with the requirements of this License.\n\n"
"Each contributor grants you a non-exclusive, worldwide, royalty-free patent license under the contributor's essential patent claims, to make, use, sell, offer for sale, import and otherwise run, modify and propagate the contents of its contributor version.\n\n"
"In the following three paragraphs, a “patent license” is any express agreement or commitment, however denominated, not to enforce a patent (such as an express permission to practice a patent or covenant not to sue for patent infringement). To “grant” such a patent license to a party means to make such an agreement or commitment not to enforce a patent against the party.\n\n"
"If you convey a covered work, knowingly relying on a patent license, and the Corresponding Source of the work is not available for anyone to copy, free of charge and under the terms of this License, through a publicly available network server or other readily accessible means, then you must either (1) cause the Corresponding Source to be so available, or (2) arrange to deprive yourself of the benefit of the patent license for this particular work, or (3) arrange, in a manner consistent with the requirements of this License, to extend the patent license to downstream recipients. “Knowingly relying” means you have actual knowledge that, but for the patent license, your conveying the covered work in a country, or your recipient's use of the covered work in a country, would infringe one or more identifiable patents in that country that you have reason to believe are valid.\n\n"
"If, pursuant to or in connection with a single transaction or arrangement, you convey, or propagate by procuring conveyance of, a covered work, and grant a patent license to some of the parties receiving the covered work authorizing them to use, propagate, modify or convey a specific copy of the covered work, then the patent license you grant is automatically extended to all recipients of the covered work and works based on it.\n\n"
"A patent license is “discriminatory” if it does not include within the scope of its coverage, prohibits the exercise of, or is conditioned on the non-exercise of one or more of the rights that are specifically granted under this License. You may not convey a covered work if you are a party to an arrangement with a third party that is in the business of distributing software, under which you make payment to the third party based on the extent of your activity of conveying the work, and under which the third party grants, to any of the parties who would receive the covered work from you, a discriminatory patent license (a) in connection with copies of the covered work conveyed by you (or copies made from those copies), or (b) primarily for and in connection with specific products or compilations that contain the covered work, unless you entered into that arrangement, or that patent license was granted, prior to 28 March 2007.\n\n"
"Nothing in this License shall be construed as excluding or limiting any implied license or other defenses to infringement that may otherwise be available to you under applicable patent law.\n\n"
"12. No Surrender of Others' Freedom.\n\n"
"If conditions are imposed on you (whether by court order, agreement or otherwise) that contradict the conditions of this License, they do not excuse you from the conditions of this License. If you cannot convey a covered work so as to satisfy simultaneously your obligations under this License and any other pertinent obligations, then as a consequence you may not convey it at all. For example, if you agree to terms that obligate you to collect a royalty for further conveying from those to whom you convey the Program, the only way you could satisfy both those terms and this License would be to refrain entirely from conveying the Program.\n\n"
"13. Use with the GNU Affero General Public License.\n\n"
"Notwithstanding any other provision of this License, you have permission to link or combine any covered work with a work licensed under version 3 of the GNU Affero General Public License into a single combined work, and to convey the resulting work. The terms of this License will continue to apply to the part which is the covered work, but the special requirements of the GNU Affero General Public License, section 13, concerning interaction through a network will apply to the combination as such.\n\n"
"14. Revised Versions of this License.\n\n"
"The Free Software Foundation may publish revised and/or new versions of the GNU General Public License from time to time. Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns.\n\n"
"Each version is given a distinguishing version number. If the Program specifies that a certain numbered version of the GNU General Public License “or any later version” applies to it, you have the option of following the terms and conditions either of that numbered version or of any later version published by the Free Software Foundation. If the Program does not specify a version number of the GNU General Public License, you may choose any version ever published by the Free Software Foundation.\n\n"
"If the Program specifies that a proxy can decide which future versions of the GNU General Public License can be used, that proxy's public statement of acceptance of a version permanently authorizes you to choose that version for the Program.\n\n"
"Later license versions may give you additional or different permissions. However, no additional obligations are imposed on any author or copyright holder as a result of your choosing to follow a later version.\n\n"
"15. Disclaimer of Warranty.\n\n"
"THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n\n"
"16. Limitation of Liability.\n\n"
"IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.\n\n"
"17. Interpretation of Sections 15 and 16.\n\n"
"If the disclaimer of warranty and limitation of liability provided above cannot be given local legal effect according to their terms, reviewing courts shall apply local law that most closely approximates an absolute waiver of all civil liability in connection with the Program, unless a warranty or assumption of liability accompanies a copy of the Program in return for a fee.\n\n"
"END OF TERMS AND CONDITIONS\n\n"
"How to Apply These Terms to Your New Programs\n\n"
"If you develop a new program, and you want it to be of the greatest possible use to the public, the best way to achieve this is to make it free software which everyone can redistribute and change under these terms.\n\n"
"To do so, attach the following notices to the program. It is safest to attach them to the start of each source file to most effectively state the exclusion of warranty; and each file should have at least the “copyright” line and a pointer to where the full notice is found.\n\n"
"DriveAssistify: a utility for managing disk operations.\n"
"Copyright (C) 2024–2025 Maksym Nazar.\n"
"Created with the assistance of ChatGPT.\n\n"
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, either version 3 of the License, or\n"
"(at your option) any later version.\n\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <https://www.gnu.org/licenses/>.\n\n"
"For inquiries, please contact us at:\n"
"Email: maximkursua@gmail.com\n";
    gtk_text_buffer_set_text(buffer, license_text, -1);

    gtk_widget_show_all(window);
}

void on_refresh_button_clicked(GtkWidget *button, gpointer user_data) {
    show_disk_list(NULL, user_data);
}

void on_exit_activate(GtkWidget *menuitem, gpointer user_data) {
    gtk_main_quit();
}

void on_terms_of_use_activate(GtkWidget *menuitem, gpointer user_data) {
    create_termsofuse_window(GTK_WIDGET(user_data));
}

void on_license_activate(GtkWidget *menuitem, gpointer user_data) {
    create_license_window(GTK_WIDGET(user_data));
}

void show_command_dialog(GtkWindow *parent, const gchar *title, const gchar *command) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title,
        parent,
        GTK_DIALOG_MODAL,
        "_Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Copy and run this command in terminal to set the boot flag:");
    gtk_container_add(GTK_CONTAINER(content_area), label);

    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), command);
    gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
    gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

void show_large_text_dialog(GtkWindow *parent, const gchar *title, const gchar *text) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        title,
        parent,
        GTK_DIALOG_MODAL,
        "_Close", GTK_RESPONSE_CLOSE,
        NULL
    );
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, 600, 400);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scrolled), textview);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, text, -1);

    gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        GtkTreeView *tree_view = GTK_TREE_VIEW(widget);
        GtkTreePath *path;
        
        if (gtk_tree_view_get_path_at_pos(tree_view, (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
            gtk_tree_view_set_cursor(tree_view, path, NULL, FALSE);
            on_row_activated(tree_view, path, NULL, user_data);
            gtk_tree_path_free(path);
            return TRUE;
        }
    }
    return FALSE;
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *tree_view;
    GtkWidget *scrolled_window;
    GtkWidget *refresh_button;
    GtkListStore *store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkWidget *menu_bar;
    GtkWidget *file_menu;
    GtkWidget *file_item;
    GtkWidget *refresh_item;
    GtkWidget *exit_item;
    GtkWidget *help_menu;
    GtkWidget *help_item;
    GtkWidget *terms_item;
    GtkWidget *license_item;

    gtk_init(&argc, &argv);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "button { border-right: 1px solid #bdbdbd; }", -1, NULL);

    GdkScreen *screen = gdk_screen_get_default();
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_object_unref(provider);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "DriveAssistify v1.7");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    menu_bar = gtk_menu_bar_new();
    file_menu = gtk_menu_new();
    help_menu = gtk_menu_new();

    file_item = gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);

    refresh_item = gtk_menu_item_new_with_label("Refresh");
    g_signal_connect(refresh_item, "activate", G_CALLBACK(on_refresh_button_clicked), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), refresh_item);

    exit_item = gtk_menu_item_new_with_label("Exit");
    g_signal_connect(exit_item, "activate", G_CALLBACK(on_exit_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), exit_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);

    help_item = gtk_menu_item_new_with_label("Help");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);

    terms_item = gtk_menu_item_new_with_label("Terms of Use");
    g_signal_connect(terms_item, "activate", G_CALLBACK(on_terms_of_use_activate), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), terms_item);

    license_item = gtk_menu_item_new_with_label("License");
    g_signal_connect(license_item, "activate", G_CALLBACK(on_license_activate), window);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), license_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);

    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    refresh_button = gtk_button_new_with_label("Refresh Disk List");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), refresh_button, FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    store = gtk_list_store_new(NUM_COLS,
        G_TYPE_STRING, // COL_NAME
        G_TYPE_STRING, // COL_SIZE
        G_TYPE_STRING, // COL_TYPE
        G_TYPE_STRING, // COL_FSTYPE
        G_TYPE_STRING, // COL_MOUNTPOINT
        G_TYPE_STRING, // COL_UUID
        G_TYPE_STRING, // COL_MODEL
        GDK_TYPE_RGBA, // COL_ROW_COLOR
        G_TYPE_STRING, // COL_FONT_COLOR
        G_TYPE_INT     // COL_WEIGHT
    );

    tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    g_signal_connect(tree_view, "button-press-event", G_CALLBACK(on_button_press), NULL);
    g_signal_connect(tree_view, "row-activated", G_CALLBACK(on_row_activated), NULL);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer,
        "text", COL_NAME,
        "cell-background-rgba", COL_ROW_COLOR,
        "foreground", COL_FONT_COLOR,
        "weight", COL_WEIGHT,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Size", renderer,
        "text", COL_SIZE,
        "cell-background-rgba", COL_ROW_COLOR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer,
        "text", COL_TYPE,
        "cell-background-rgba", COL_ROW_COLOR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("File System", renderer,
        "text", COL_FSTYPE,
        "cell-background-rgba", COL_ROW_COLOR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Mount Point", renderer,
        "text", COL_MOUNTPOINT,
        "cell-background-rgba", COL_ROW_COLOR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("UUID", renderer,
        "text", COL_UUID,
        "cell-background-rgba", COL_ROW_COLOR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Model", renderer,
        "text", COL_MODEL,
        "cell-background-rgba", COL_ROW_COLOR,
        NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);

    gtk_container_add(GTK_CONTAINER(scrolled_window), tree_view);

    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_button_clicked), tree_view);

    show_disk_list(NULL, tree_view);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}




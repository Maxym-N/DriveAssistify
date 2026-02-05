/*
 * DriveAssistify: a utility for managing disk operations.
 *
 * Version: 1.8
 * 
 * Copyright (C) 2024–2025 Maksym Nazar.
 * Created with the assistance of ChatGPT, Perplexity, and Claude.
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
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pango/pango.h>
#if defined(GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#endif

static const char icon_base64[] =
    "iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAAk1BMVEVHcEw3QFI6Q1NUjFOL314+SFiD1Vg8RVmX32By0V1cyF9myV06Q1RiyFw7Q1aK2Fs6RFWE12I4SFCj6Gc5QlQ8RVc2P1Foylphx1uBjaBLU2WF1VdvzVp90lh1z1gwOktXxF92gpRDTF1iq2JYYnRmcYP3+vduuGBUi1ZWoVhIblajsMLF6cKS15Chrb+cqLqQm63faOv+AAAAFHRSTlMASmMKHZe5qv4/5Znlf33g82Eid8ScxAsAAAQOSURBVFjD7ZfZeqM4EIXp4GVs4rid0QIyYBBgA17i93+6qSpJLAn2pGdu+qKPaTrwcX4OpRKWPe+Pfjf5s5H8XwYs/vqk7SLYzX4FsP8ipPzt/w+AoXwX8Qiw329X/xmQ2BC77wISZzF/4kGS4Lb7HsCayJOYvfkr2T98Ct8KAPbSZLtAbZPeDtpODqi//Mk7nez1+1qQXuebbedP9sEU4KW3M3Zy19YHlDiwd3/TExKM4P/oRIA516V2hGIEEPCBi1ZwnNInCcZ3fHEARYeqyvsEwgrvYgj4b+sjIIrutPE3A+DWHkWfAIwxAniBIaRpukLAx+18voFGgDKKEJBS1jSt0Y0iwMyeTdOAAPf89pHfz0MA+QlAVyaFJjs3AG+RWi0IkOuPu86HABU5QGIQRW7HxQB2DhCbBNHtFt3PugfkUZ+AVOQV2qEItFPOn/oIOBv1CcwDjAGRIsABagmbO5/O/DeeW/G5bwEuQOUACQAqKsKBBvMQuwSrVzzLbYnZOwKYdgF4MUiQuwQIkIYQx6ufXHeiEs37J1DaJogxQaR5D4idAKA68TEg59XJ3ggTmCJAQ0MReQeYTQGqAcDoE4CpDuA/SRBx5QAxAZgBwDC07vzamwAw20ZRyYohgHcA1pizUi6+ApYwjJGr4gCQ0zCaR9AyluSXgfc6AGgHqLpGchcCoOwBjTT3l3LnvbKJBLYR8hIT4N0QwB0AKiCtwtkogQMwW0YlCinNxYUNgADjx10Ik3EiAVym8W2iGAKkSVCaaQSNVKeyC7B6BIAy5NCYBoBqaCJo1Taxyx+GYeN50zWwc5/pkwPE5s2WUvAQdwhQBtDWoBYB3NbATi9RdQBjoduSldQKBNTUbTI+1d0jqKo0JVdF7yaAc9O+PgBg1jVrLE9KvFCCnCYepBBNOK0MNviqEP5sbbzmc9IGQDOXPwJkYZZl4anFjvDXZO50coASuymHlpt2Z1lN7za2GfjTy1XKjQVEpSrhLegAmbMbNa2xMx72N48vx6MM174FmE5kbdEUuNn/mgaGq9VCwCPCzGHtYJDAf4HarrwlGwDE4atKrSCfrsqK1f0gWX+4wWHs3khiDDBHZVmqqlIVr3gD3uslBv8V/PSUgZlMWENsJjFACF6VcCjAW5a4qxgCjsdjTH5JFQ68t8H6ghGCCbOD1DiZOHSs0hxf4jV0FGQ/XpFiRiiAJY6NwCzAfZ/17xMn0WJodIOfxifMcOn242VKS/sgPQByZY5wpe4APV7Bzj8B6PslIN/1crUdkgXec4CwfmEJ7+tspPWTJfRcCNHf3gFmI8L62QL6bVBLBBHO94aE9dNfEu+vzAUQFsCWuEzcWMR68y8/AQYLytHC0vNXmyDYrPw/vxB/H/0DstwCgix/EeQAAAAASUVORK5CYII=";

static void set_window_icon(GtkWidget *window) {
    g_print("Attempting to set window icon...\n");
    GdkPixbuf *icon = NULL;
    GError *error = NULL;
    gsize data_len;

    guchar* data = g_base64_decode(icon_base64, &data_len);
    if (!data) {
        g_warning("Failed to decode Base64 data\n");
        return;
    }

    GInputStream *stream = g_memory_input_stream_new_from_data(data, data_len, g_free);
    if (!stream) {
        g_warning("Failed to create input stream\n");
        g_free(data);
        return;
    }
    g_print("Input stream created successfully\n");

    icon = gdk_pixbuf_new_from_stream(stream, NULL, &error);
    g_object_unref(stream);

    if (error) {
        g_warning("Error loading icon: %s", error->message);
        g_error_free(error);
        return;
    }

    if (icon) {
        int width = gdk_pixbuf_get_width(icon);
        int height = gdk_pixbuf_get_height(icon);
        g_print("Original icon size: %dx%d\n", width, height);

        GdkPixbuf *scaled_icon = gdk_pixbuf_scale_simple(icon, 
            width,
            height,
            GDK_INTERP_BILINEAR);

        if (scaled_icon) {
            gtk_window_set_icon(GTK_WINDOW(window), scaled_icon);
            g_object_unref(scaled_icon);
            g_print("Icon set successfully at %dx%d\n", width, height);
        } else {
            g_warning("Failed to process icon\n");
        }
        g_object_unref(icon);
    } else {
        g_warning("Failed to create icon from data\n");
    }
}

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

typedef struct {
    GtkWidget *disk_areas_window;
    gchar *device_path;
    GtkTreeView *main_tree_view;
} DiskAreasRefreshData;

static void set_window_icon(GtkWidget *window);
static gchar *get_disk_from_partition(const gchar *partition);
static gchar *get_base_device(const gchar *dev);
static gboolean refresh_disk_list_delayed(gpointer user_data);
static void on_external_terminal_exited(GPid pid, gint status, gpointer user_data);
static void on_terminal_child_exited_disk_areas(VteTerminal *terminal, gint status, gpointer user_data);
static void on_mount_child_exited(VteTerminal *terminal, gint status, gpointer user_data);
static void on_benchmark_bytes_changed(GtkEditable *editable, gpointer user_data);
static void on_benchmark_mib_changed(GtkEditable *editable, gpointer user_data);
static void on_benchmark_gib_changed(GtkEditable *editable, gpointer user_data);
static gboolean check_tmp_space_for_size(long long required_gib);
static void on_entry_bytes_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_mib_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_gib_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_sectors_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_start_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_end_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_bytes_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_mib_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_gib_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_sectors_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_start_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_end_changed(GtkEditable *editable, gpointer user_data);
static void on_resize_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void on_resize_dialog_destroy(GtkWidget *dialog, gpointer user_data);
static void update_cluster_options_cb(GtkComboBoxText *fs_combo, gpointer user_data);

void set_row_color_based_on_type(GtkTreeViewColumn *col, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
void get_value(const char *src, const char *key, char *buf, size_t buflen);
int compare_rows(const void *a, const void *b);
void show_disk_list(GtkWidget *widget, gpointer tree_view);
void on_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void run_command_in_terminal(GtkTreeView *tree_view, const gchar *command_template);
void run_command_simple(const gchar *command, GtkWidget *parent_window, GtkWidget *disk_areas_window, const gchar *device_path, GtkTreeView *main_tree_view);
void run_command(GtkTreeView *tree_view, const gchar *command);
void on_device_info_activate(GtkWidget *menuitem, gpointer user_data);
void on_show_disk_areas_activate(GtkWidget *menuitem, gpointer user_data);
void on_create_fs_clicked(GtkWidget *button, gpointer user_data);
void on_smartctl_activate(GtkWidget *menuitem, gpointer user_data);
void on_disk_read_benchmark_activate(GtkWidget *menuitem, gpointer user_data);
void on_disk_file_write_benchmark_activate(GtkWidget *menuitem, gpointer user_data);
void on_disk_raw_write_benchmark_activate(GtkWidget *button, gpointer user_data);
void on_auto_fsck_activate(GtkWidget *menuitem, gpointer user_data);
void on_e2fsck_activate(GtkWidget *menuitem, gpointer user_data);
void on_ext_repair_deep_activate(GtkWidget *menuitem, gpointer user_data);
void on_fat32fix_activate(GtkWidget *menuitem, gpointer user_data);
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
void on_grub_uefi_install_activate(GtkWidget *menuitem, gpointer user_data);
void on_grub_mbr_install_activate(GtkWidget *menuitem, gpointer user_data);
void on_toggle_boot_flag_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_copy_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_restore_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_delete_partition_table_activate(GtkWidget *menuitem, gpointer user_data);
void on_delete_partition_activate(GtkWidget *menuitem, gpointer user_data);
void on_shred_fs_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_erase_activate(GtkWidget *menuitem, gpointer user_data);
void on_dd_multiple_erase_activate(GtkWidget *menuitem, gpointer user_data);
void clean_string(char *str);
void create_termsofuse_window(GtkWidget *parent);
void create_license_window(GtkWidget *parent);
void on_refresh_button_clicked(GtkWidget *button, gpointer user_data);
void on_exit_activate(GtkWidget *menuitem, gpointer user_data);
void on_terms_of_use_activate(GtkWidget *menuitem, gpointer user_data);
void on_license_activate(GtkWidget *menuitem, gpointer user_data);
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

static gboolean refresh_disk_list_delayed(gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    if (tree_view && GTK_IS_TREE_VIEW(tree_view)) {
        show_disk_list(NULL, tree_view);
    }
    return FALSE;
}

static void on_external_terminal_exited(GPid pid, gint status, gpointer user_data) {
    g_spawn_close_pid(pid);

    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    if (tree_view && GTK_IS_TREE_VIEW(tree_view)) {
        g_timeout_add(1000, refresh_disk_list_delayed, g_object_ref(tree_view));
    }
}

void run_command_in_terminal(GtkTreeView *tree_view, const gchar *cmd_template) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;

    gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);
    if (!partition_name || strlen(partition_name) == 0) {
        g_free(partition_name);
        return;
    }
    
    gchar *device_path = g_strdup_printf("/dev/%s", partition_name);
    
    gchar *cmd;
    if (strchr(cmd_template, '%'))
        cmd = g_strdup_printf(cmd_template, device_path);
    else
        cmd = g_strdup(cmd_template);

    const gchar *terminals[] = {
        "x-terminal-emulator", "konsole", "gnome-terminal --",
        "xfce4-terminal", "lxterminal", "mate-terminal",
        "alacritty", "kitty", "xterm", NULL
    };

    gchar *full_cmd = g_strdup_printf("%s; sleep 0; echo; echo 'Press Enter to close...'; read -p ''", cmd);
    gchar *quoted_cmd = g_shell_quote(full_cmd);

    gboolean found = FALSE;
    GPid child_pid = 0;

    for (int i = 0; terminals[i]; i++) {
        if (g_find_program_in_path(terminals[i])) {
            gchar *term_cmd = NULL;

            if (g_strcmp0(terminals[i], "gnome-terminal --") == 0)
                term_cmd = g_strdup_printf("gnome-terminal -- bash -c %s", quoted_cmd);
            else if (g_strcmp0(terminals[i], "konsole") == 0)
                term_cmd = g_strdup_printf("konsole -e sh -c %s", quoted_cmd);
            else
                term_cmd = g_strdup_printf("%s -e sh -c %s", terminals[i], quoted_cmd);

            gchar **argv;
            GError *error = NULL;

            if (g_shell_parse_argv(term_cmd, NULL, &argv, &error)) {
                if (g_spawn_async(NULL, argv, NULL,
                                  G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                                  NULL, NULL, &child_pid, &error)) {
                    g_child_watch_add(child_pid, on_external_terminal_exited, g_object_ref(tree_view));
                    found = TRUE;
                } else if (error) {
                    g_warning("Failed to spawn terminal: %s", error->message);
                    g_clear_error(&error);
                }
                g_strfreev(argv);
            } else if (error) {
                g_warning("Failed to parse command: %s", error->message);
                g_clear_error(&error);
            }

            g_free(term_cmd);
            break;
        }
    }

    if (!found) {
        GtkWidget *err = gtk_message_dialog_new(
            GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(tree_view))),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "No compatible terminal emulator found.\nPlease install xterm, gnome-terminal, or similar."
        );
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
    }

    g_free(quoted_cmd);
    g_free(full_cmd);
    g_free(cmd);
    g_free(device_path);
    g_free(partition_name);
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
                "=== SECTOR INFO (PARSED FROM FDISK) ===\nStart sector: %s\nEnd sector: %s\nTotal sectors: %s\n",
                start_sector, end_sector, total_sectors);
        } else if (strlen(total_sectors) > 0) {
            g_string_append_printf(sector_info,
                "=== SECTOR INFO (PARSED FROM FDISK) ===\nTotal sectors: %s\n", total_sectors);
        }

        gchar fs_type[64] = "";
        gchar *fstype_cmd = g_strdup_printf("lsblk -no FSTYPE %s", device_path);
        FILE *fst_fp = popen(fstype_cmd, "r");
        if (fst_fp && fgets(fs_type, sizeof(fs_type), fst_fp)) {
            g_strstrip(fs_type);
        }
        if (fst_fp) pclose(fst_fp);
        g_free(fstype_cmd);

        GString *cluster_info = g_string_new("");
        if (strlen(fs_type) > 0) {
            gchar cmd[256];
            FILE *fp2;
            char buf[256];

            if (g_str_has_prefix(fs_type, "ext")) {
                g_snprintf(cmd, sizeof(cmd),
                           "sudo dumpe2fs -h %s 2>&1 | grep 'Block size'", device_path);
                fp2 = popen(cmd, "r");
                if (fp2 && fgets(buf, sizeof(buf), fp2)) {
                    g_string_append_printf(cluster_info, "Block size (cluster): %s", g_strstrip(buf));
                }
                if (fp2) pclose(fp2);
            }
            else if (g_strcmp0(fs_type, "vfat") == 0 || g_strcmp0(fs_type, "fat16") == 0 || g_strcmp0(fs_type, "fat32") == 0) {
                g_snprintf(cmd, sizeof(cmd),
                           "sudo dosfsck -v %s 2>/dev/null | grep 'bytes per cluster'", device_path);
                fp2 = popen(cmd, "r");
                if (fp2 && fgets(buf, sizeof(buf), fp2)) {
                    g_string_append_printf(cluster_info, "Cluster size: %s", g_strstrip(buf));
                }
                if (fp2) pclose(fp2);
            }
            else if (g_strcmp0(fs_type, "exfat") == 0) {
                g_snprintf(cmd, sizeof(cmd),
                           "sudo dumpexfat -i %s 2>/dev/null | grep 'Cluster Size'", device_path);
                fp2 = popen(cmd, "r");
                if (fp2 && fgets(buf, sizeof(buf), fp2)) {
                    g_string_append_printf(cluster_info, "Cluster size: %s", g_strstrip(buf));
                }
                if (fp2) pclose(fp2);
            }
            else if (g_strcmp0(fs_type, "ntfs") == 0) {
                g_snprintf(cmd, sizeof(cmd),
                           "sudo ntfsinfo -m %s 2>/dev/null | grep 'Cluster Size'", device_path);
                fp2 = popen(cmd, "r");
                if (fp2 && fgets(buf, sizeof(buf), fp2)) {
                    g_string_append_printf(cluster_info, "Cluster size: %s", g_strstrip(buf));
                }
                if (fp2) pclose(fp2);
            }
        }

        gchar *command = g_strdup_printf(
            "echo '=== LSBLK ==='; lsblk -f %s; "
            "echo ''; echo '=== FDISK ==='; fdisk -l %s; "
            "echo ''; echo '=== PARTED ==='; parted %s print; "
            "echo ''; echo '=== BLKID ==='; blkid %s",
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

            if (cluster_info->len > 0) {
                final_text = g_strconcat(final_text,
                                         "\n=== FILESYSTEM CLUSTER INFO (PARSED FROM DUMPE2FS/DOSFSCK/NTFSINFO/DUMPEXFAT) ===\n",
                                         cluster_info->str,
                                         NULL);
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

        g_string_free(cluster_info, TRUE);
        g_string_free(sector_info, TRUE);
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
                    "This function is intended only for entire disks (e.g., /dev/sdb), not for partitions (e.g., /dev/sdb1). Please select a whole disk (not a partition)."
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
                        gchar *part_dev;
                        if (g_str_has_prefix(disk_name, "nvme") || g_str_has_prefix(disk_name, "mmcblk")) {
                            part_dev = g_strdup_printf("%sp%s", device_path, num);
                        } else {
                            part_dev = g_strdup_printf("%s%s", device_path, num);
                        }
                        
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

            g_object_set_data(G_OBJECT(create_fs_btn), "disk_path", g_strdup(device_path));
            g_object_set_data(G_OBJECT(create_fs_btn), "main_tree_view", tree_view);
            g_object_set_data(G_OBJECT(create_fs_btn), "disk_areas_window", window);
            g_signal_connect(create_fs_btn, "clicked", G_CALLBACK(on_create_fs_clicked), tree);

            gtk_widget_show_all(window);
        }

        g_free(command);
        g_free(device_path);
        g_free(disk_name);
    }
}

typedef struct {
    GtkWidget *entry_bytes;
    GtkWidget *entry_mib;
    GtkWidget *entry_gib;
    GtkWidget *entry_sectors;
    GtkWidget *entry_start;
    GtkWidget *entry_end;
    GtkWidget *label_free;
    long long orig_free;
    long long start_mib;
    long long end_mib;
    int sector_size;
    gboolean updating;
} CreateFSWidgets;

static void on_create_entry_mib_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_start_changed(GtkEditable *editable, gpointer user_data);
static void on_create_entry_end_changed(GtkEditable *editable, gpointer user_data);

static void on_create_entry_bytes_changed(GtkEditable *editable, gpointer user_data) {
    CreateFSWidgets *w = (CreateFSWidgets*)user_data;
    if (w->updating) return;
    w->updating = TRUE;

    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (!text || strlen(text) == 0) { w->updating = FALSE; return; }

    long long bytes = atoll(text);
    long long mib = bytes / (1024*1024);
    double gib = (double)bytes / (1024.0*1024.0*1024.0);
    long long sectors = (bytes + w->sector_size - 1) / w->sector_size;

    g_signal_handlers_block_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
    g_signal_handlers_block_by_func(w->entry_gib, NULL, w);
    g_signal_handlers_block_by_func(w->entry_sectors, NULL, w);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", mib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_mib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%.2f", gib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_gib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", sectors);
    gtk_entry_set_text(GTK_ENTRY(w->entry_sectors), tmp);

    g_signal_handlers_unblock_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_gib, NULL, w);
    g_signal_handlers_unblock_by_func(w->entry_sectors, NULL, w);

    w->updating = FALSE;
}

static void on_create_entry_mib_changed(GtkEditable *editable, gpointer user_data) {
    CreateFSWidgets *w = (CreateFSWidgets*)user_data;
    if (w->updating) return;
    w->updating = TRUE;

    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (!text || strlen(text) == 0) { w->updating = FALSE; return; }

    long long input_mib = atoll(text);
    if (input_mib < 0) input_mib = 0;

    long long bytes = input_mib * 1024LL * 1024LL;
    double gib = (double)input_mib / 1024.0;
    long long sectors = (bytes + w->sector_size - 1) / w->sector_size;

    g_signal_handlers_block_by_func(w->entry_bytes, G_CALLBACK(on_create_entry_bytes_changed), w);
    g_signal_handlers_block_by_func(w->entry_gib, NULL, w);
    g_signal_handlers_block_by_func(w->entry_sectors, NULL, w);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", bytes);
    gtk_entry_set_text(GTK_ENTRY(w->entry_bytes), tmp);
    g_snprintf(tmp, sizeof(tmp), "%.2f", gib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_gib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", sectors);
    gtk_entry_set_text(GTK_ENTRY(w->entry_sectors), tmp);

    g_signal_handlers_unblock_by_func(w->entry_bytes, G_CALLBACK(on_create_entry_bytes_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_gib, NULL, w);
    g_signal_handlers_unblock_by_func(w->entry_sectors, NULL, w);
    
    if (w->label_free) {
		    long long left = w->orig_free - input_mib;
		    if (left < 0) left = 0;
		    char lbl[128];
		    g_snprintf(lbl, sizeof(lbl), "Free space after creation: %lld MiB (%.2f GiB)", 
               left, (double)left / 1024.0);
		    gtk_label_set_text(GTK_LABEL(w->label_free), lbl);
}

    w->updating = FALSE;
}

static void on_create_entry_start_changed(GtkEditable *editable, gpointer user_data) {
    CreateFSWidgets *w = (CreateFSWidgets*)user_data;
    if (w->updating) return;
    w->updating = TRUE;

    const char *start_text = gtk_entry_get_text(GTK_ENTRY(w->entry_start));
    const char *mib_text = gtk_entry_get_text(GTK_ENTRY(w->entry_mib));
    long long start = atoll(start_text);
    long long size_mib = atoll(mib_text);

    long long new_end = start + size_mib;
    if (new_end > w->end_mib) new_end = w->end_mib;

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", new_end);
    
    g_signal_handlers_block_by_func(w->entry_end, G_CALLBACK(on_create_entry_end_changed), w);
    gtk_entry_set_text(GTK_ENTRY(w->entry_end), tmp);
    g_signal_handlers_unblock_by_func(w->entry_end, G_CALLBACK(on_create_entry_end_changed), w);

    w->updating = FALSE;
}

static void on_create_entry_end_changed(GtkEditable *editable, gpointer user_data) {
    CreateFSWidgets *w = (CreateFSWidgets*)user_data;
    if (w->updating) return;
    w->updating = TRUE;

    const char *start_text = gtk_entry_get_text(GTK_ENTRY(w->entry_start));
    const char *end_text = gtk_entry_get_text(GTK_ENTRY(w->entry_end));
    long long start = atoll(start_text);
    long long end = atoll(end_text);

    if (end > start) {
        long long size_mib = end - start;
        char tmp[64];
        g_snprintf(tmp, sizeof(tmp), "%lld", size_mib);
        
        g_signal_handlers_block_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
        gtk_entry_set_text(GTK_ENTRY(w->entry_mib), tmp);
        g_signal_handlers_unblock_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);

        on_create_entry_mib_changed(GTK_EDITABLE(w->entry_mib), w);
    }

    w->updating = FALSE;
}

static void on_terminal_child_exited_disk_areas(VteTerminal *terminal, gint status, gpointer user_data) {
    GtkWindow *window = GTK_WINDOW(user_data);
    gtk_window_set_title(window, "Command Finished");
    
    DiskAreasRefreshData *data = g_object_get_data(G_OBJECT(window), "refresh_data");
    if (data && data->disk_areas_window) {
        gtk_widget_destroy(data->disk_areas_window);

        if (data->main_tree_view && GTK_IS_TREE_VIEW(data->main_tree_view)) {
            g_timeout_add(100, refresh_disk_list_delayed, data->main_tree_view);
        }

        GtkWidget *info = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Operation completed successfully!\n\n"
            "The main window will refresh automatically.\n"
            "Please open 'Show Filesystems and Free Space' again to see updated partition information."
        );
        gtk_dialog_run(GTK_DIALOG(info));
        gtk_widget_destroy(info);

        g_free(data->device_path);
        g_free(data);
    }
}

void run_command_simple(const gchar *command, GtkWidget *parent_window, GtkWidget *disk_areas_window, const gchar *device_path, GtkTreeView *main_tree_view) {
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Command Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    if (parent_window) {
        gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(gtk_widget_get_toplevel(parent_window)));
    }

    GtkWidget *terminal = vte_terminal_new();
    gtk_container_add(GTK_CONTAINER(window), terminal);

    gchar *cmd_copy = g_strdup(command);
    
    if (disk_areas_window && device_path) {
        DiskAreasRefreshData *data = g_new0(DiskAreasRefreshData, 1);
        data->disk_areas_window = disk_areas_window;
        data->device_path = g_strdup(device_path);
        data->main_tree_view = main_tree_view;
        g_object_set_data(G_OBJECT(window), "refresh_data", data);
        g_signal_connect(terminal, "child-exited", G_CALLBACK(on_terminal_child_exited_disk_areas), window);
    } else {
        g_signal_connect_swapped(terminal, "child-exited", 
                                 G_CALLBACK(gtk_window_set_title), window);
    }

    vte_terminal_spawn_async(VTE_TERMINAL(terminal), VTE_PTY_DEFAULT,
                             NULL, (char *[]){"/bin/sh", "-c", cmd_copy, NULL},
                             NULL, 0, NULL, NULL, NULL, -1, NULL, NULL, NULL);

    g_free(cmd_copy);
    gtk_widget_show_all(window);
}

typedef struct {
    GtkWidget *cluster_label;
    GtkWidget *cluster_combo;
} ClusterUpdateData;

static void
update_cluster_options_cb(GtkComboBoxText *fs_combo, gpointer user_data)
{
    ClusterUpdateData *d = (ClusterUpdateData*) user_data;
    GtkComboBoxText *cluster_combo = GTK_COMBO_BOX_TEXT(d->cluster_combo);
    const gchar *fs = gtk_combo_box_text_get_active_text(fs_combo);

    gtk_combo_box_text_remove_all(cluster_combo);

    if (!fs) {
        gtk_widget_hide(d->cluster_label);
        gtk_widget_hide(d->cluster_combo);
        return;
    }

    if (g_str_has_prefix(fs, "ext")) {
        const int blocks[] = {1024, 2048, 4096, 8192, 16384, 32768, 65536};
        const int default_block = 4096;
        for (int i = 0; i < 7; i++) {
            gchar tmp[32];
            g_snprintf(tmp, sizeof(tmp), "%d%s", blocks[i], (blocks[i]==default_block ? " (default)" : ""));
            gtk_combo_box_text_append_text(cluster_combo, tmp);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(cluster_combo), 2);
        gtk_widget_show(d->cluster_label);
        gtk_widget_show(d->cluster_combo);
        return;
    }

    if (g_strcmp0(fs, "fat32") == 0) {
        const int sectors[] = {1, 2, 4, 8, 16, 32, 64, 128};
        const int default_sector = 1;
        for (int i = 0; i < 8; i++) {
            gchar tmp[32];
            g_snprintf(tmp, sizeof(tmp), "%d%s", sectors[i], (sectors[i]==default_sector ? " (default)" : ""));
            gtk_combo_box_text_append_text(cluster_combo, tmp);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(cluster_combo), 0);
        gtk_widget_show(d->cluster_label);
        gtk_widget_show(d->cluster_combo);
        return;
    }

    if (g_strcmp0(fs, "exfat") == 0) {
        const int clusters[] =
            {1, 2, 4, 8, 16, 32, 64, 128,
             256, 512, 1024, 2048, 4096,
             8192, 16384, 32768};
        const int default_cluster = 128;
        for (int i = 0; i < 16; i++) {
            gchar tmp[32];
            g_snprintf(tmp, sizeof(tmp), "%d%s", clusters[i], (clusters[i]==default_cluster ? " (default)" : ""));
            gtk_combo_box_text_append_text(cluster_combo, tmp);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(cluster_combo), 7);
        gtk_widget_show(d->cluster_label);
        gtk_widget_show(d->cluster_combo);
        return;
    }

    if (g_str_has_prefix(fs, "ntfs") || g_strcmp0(fs, "ntfs") == 0) {
        const int clusters[] = {512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};
        const int default_cluster = 4096;
        for (int i = 0; i < 8; i++) {
            gchar tmp[32];
            g_snprintf(tmp, sizeof(tmp), "%d%s", clusters[i], (clusters[i]==default_cluster ? " (default)" : ""));
            gtk_combo_box_text_append_text(cluster_combo, tmp);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(cluster_combo), 3);
        gtk_widget_show(d->cluster_label);
        gtk_widget_show(d->cluster_combo);
        return;
    }

    gtk_widget_hide(d->cluster_label);
    gtk_widget_hide(d->cluster_combo);
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

    long long start_mib = atoll(start);
    long long end_mib = atoll(end);
    long long free_size = end_mib - start_mib;

    int phys_sector_size = 512;
    {
        gchar *ss_cmd = g_strdup_printf("blockdev --getss %s", disk_path);
        FILE *ss_fp = popen(ss_cmd, "r");
        if (ss_fp) {
            if (fscanf(ss_fp, "%d", &phys_sector_size) != 1)
                phys_sector_size = 512;
            pclose(ss_fp);
        }
        g_free(ss_cmd);
    }

    GtkWidget *size_dialog = gtk_dialog_new_with_buttons(
        "Specify Partition Size",
        NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Next", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(size_dialog), 500, 370);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(size_dialog));

    GtkWidget *info_label = gtk_label_new(NULL);
    gchar *info_text = g_strdup_printf(
        "Free space available: %lld MiB (%.2f GiB)\n"
        "Start position: %lld MiB\nEnd position: %lld MiB",
        free_size, (double)free_size / 1024.0, start_mib, end_mib);
    gtk_label_set_text(GTK_LABEL(info_label), info_text);
    g_free(info_text);
    gtk_box_pack_start(GTK_BOX(content_area), info_label, FALSE, FALSE, 5);

    GtkWidget *entry_bytes = gtk_entry_new();
    GtkWidget *entry_mib = gtk_entry_new();
    GtkWidget *entry_gib = gtk_entry_new();
    GtkWidget *entry_sectors = gtk_entry_new();
    GtkWidget *label_free = gtk_label_new(NULL);
    GtkWidget *align_check = gtk_check_button_new_with_label(
        "Align partition (start at 1 MiB boundary, recommended)"
    );
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(align_check), TRUE);

    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in Bytes:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_bytes, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in MiB:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_mib, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in GiB:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_gib, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in Sectors:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_sectors, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), label_free, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(content_area), align_check, FALSE, FALSE, 5);

    CreateFSWidgets *widgets = g_new0(CreateFSWidgets, 1);
    widgets->entry_bytes = entry_bytes;
    widgets->entry_mib = entry_mib;
    widgets->entry_gib = entry_gib;
    widgets->entry_sectors = entry_sectors;
    widgets->label_free = label_free;
    widgets->orig_free = free_size;
    widgets->start_mib = start_mib;
    widgets->end_mib = end_mib;
    widgets->sector_size = phys_sector_size;
    widgets->updating = FALSE;

    g_signal_connect(entry_bytes, "changed", G_CALLBACK(on_create_entry_bytes_changed), widgets);
    g_signal_connect(entry_mib, "changed", G_CALLBACK(on_create_entry_mib_changed), widgets);
    g_signal_connect(entry_gib, "changed", G_CALLBACK(on_entry_gib_changed), widgets);
    g_signal_connect(entry_sectors, "changed", G_CALLBACK(on_entry_sectors_changed), widgets);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", free_size);
    gtk_entry_set_text(GTK_ENTRY(entry_mib), tmp);
    on_create_entry_mib_changed(GTK_EDITABLE(entry_mib), widgets);

    gtk_widget_show_all(size_dialog);

    gint size_response = gtk_dialog_run(GTK_DIALOG(size_dialog));
    long long target_size_mib = 0;
    gboolean align = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(align_check));

    if (size_response == GTK_RESPONSE_ACCEPT) {
        const gchar *mib_text = gtk_entry_get_text(GTK_ENTRY(entry_mib));
        target_size_mib = atoll(mib_text);
        if (target_size_mib > free_size) target_size_mib = free_size;
    }

    g_free(widgets);
    gtk_widget_destroy(size_dialog);

    if (size_response != GTK_RESPONSE_ACCEPT || target_size_mib <= 0) {
        g_free(start); g_free(end); if (type) g_free(type);
        g_free(disk_path);
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
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 140);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext4");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext3");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ntfs (quick format)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ntfs (full zeroing)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "exfat");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fat32");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    GtkWidget *cluster_label = gtk_label_new("Cluster / Block size:");
    GtkWidget *cluster_combo = gtk_combo_box_text_new();

    GtkWidget *sector_label = gtk_label_new("Logical sector size (bytes):");
    GtkWidget *sector_combo = gtk_combo_box_text_new();

    const int sector_options[] = {512,1024,2048,4096,8192,16384};

    int active_index = 0;
    for (int i = 0; i < 6; i++) {
        if (phys_sector_size == sector_options[i]) {
            g_snprintf(tmp, sizeof(tmp), "%d (default)", sector_options[i]);
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sector_combo), tmp);
            active_index = i;
        } else {
            g_snprintf(tmp, sizeof(tmp), "%d", sector_options[i]);
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sector_combo), tmp);
        }
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(sector_combo), active_index);

    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Select filesystem type:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), combo, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), sector_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), sector_combo, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), cluster_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), cluster_combo, FALSE, FALSE, 2);

    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new(""), FALSE, FALSE, 5);

    gtk_widget_show_all(dialog);

    ClusterUpdateData cud;
    cud.cluster_label = cluster_label;
    cud.cluster_combo = cluster_combo;

    g_signal_connect(combo, "changed", G_CALLBACK(update_cluster_options_cb), &cud);
    update_cluster_options_cb(GTK_COMBO_BOX_TEXT(combo), &cud);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const gchar *fs_display = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        const gchar *cluster_sel = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cluster_combo));
        const gchar *sector_sel_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(sector_combo));

        gboolean is_ntfs_quick = g_strcmp0(fs_display, "ntfs (quick format)") == 0;
        gboolean is_ntfs_full = g_strcmp0(fs_display, "ntfs (full zeroing)") == 0;
        const gchar *fs_type = is_ntfs_quick || is_ntfs_full ? "ntfs" : fs_display;

        int chosen_sector = 512;
        if (sector_sel_str) {
            char *clean = g_strdup(sector_sel_str);
            char *p = strstr(clean, " (default)");
            if (p) *p = 0;
            chosen_sector = atoi(clean);
            g_free(clean);
        }

        int cluster_val = 0;
        if (cluster_sel) {
            char *clean = g_strdup(cluster_sel);
            char *p = strchr(clean, ' ');
            if (p) *p = 0;
            cluster_val = atoi(clean);
            g_free(clean);
        } else {
            if (g_strcmp0(fs_type, "ext4")==0 || g_strcmp0(fs_type,"ext3")==0) cluster_val = 4096;
            else if (g_strcmp0(fs_type,"ext2")==0) cluster_val = 1024;
            else if (g_strcmp0(fs_type,"exfat")==0) cluster_val = 8;
            else if (g_strcmp0(fs_type,"fat32")==0) cluster_val = 1;
        }

        long long actual_start = start_mib < 1 && align ? 1 : start_mib;
        long long actual_end = actual_start + target_size_mib;
        if (actual_end > end_mib) actual_end = end_mib;

        long long start_sector = (actual_start * 1024LL * 1024LL) / chosen_sector;
        long long end_sector = (actual_end * 1024LL * 1024LL) / chosen_sector;

        if (align) {
            long long sectors_per_1MiB = 1048576LL / chosen_sector;
            if (start_sector < sectors_per_1MiB) start_sector = sectors_per_1MiB;
            else if (start_sector % sectors_per_1MiB != 0)
                start_sector = ((start_sector / sectors_per_1MiB) + 1) * sectors_per_1MiB;
        }

        gchar *mkpart_cmd = g_strdup_printf("sudo parted -s %s mkpart primary %llds %llds",
                                            disk_path, start_sector, end_sector - 1);

        gchar *find_part_cmd = g_strdup_printf(
            "lsblk -ln -o NAME %s 2>/dev/null | tail -1 | awk '{print \"/dev/\" $1}' || "
            "parted -sm %s unit s print 2>/dev/null | grep '^[0-9]' | awk -F: '$2 ~ /^%llds/ {print \"%s\" $1; exit}'",
            disk_path, disk_path, start_sector, disk_path);

        gchar *mkfs_cmd = NULL;
        if (g_strcmp0(fs_type, "ext4")==0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ext4 -F -b %d \"$NEW_PART\"", cluster_val);
        else if (g_strcmp0(fs_type,"ext3")==0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ext3 -F -b %d \"$NEW_PART\"", cluster_val);
        else if (g_strcmp0(fs_type,"ext2")==0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.ext2 -F -b %d \"$NEW_PART\"", cluster_val);
        else if (is_ntfs_quick) {
            if (cluster_val > 0)
                mkfs_cmd = g_strdup_printf("sudo mkfs.ntfs -f -c %d -Q \"$NEW_PART\"", cluster_val);
            else
                mkfs_cmd = g_strdup_printf("sudo mkfs.ntfs -f -Q \"$NEW_PART\"");
        }
        else if (is_ntfs_full) {
            if (cluster_val > 0)
                mkfs_cmd = g_strdup_printf("sudo mkfs.ntfs -F -c %d \"$NEW_PART\"", cluster_val);
            else
                mkfs_cmd = g_strdup_printf("sudo mkfs.ntfs -F \"$NEW_PART\"");
        }
        else if (g_strcmp0(fs_type,"exfat")==0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.exfat -s %d \"$NEW_PART\"", cluster_val);
        else if (g_strcmp0(fs_type,"fat32")==0)
            mkfs_cmd = g_strdup_printf("sudo mkfs.vfat -F 32 -s %d \"$NEW_PART\"", cluster_val);

        gchar *cluster_display = cluster_val > 0 ? g_strdup_printf("%d", cluster_val) : g_strdup("default");
        gchar *format_desc = g_strdup("");

        gchar *warn_text = g_strdup_printf(
            "WARNING: This will create a new partition and destroy any data in the selected area!\n\n"
            "Filesystem type: %s %s\nLogical sector size: %s\nCluster/Block: %s\nSize: %lld MiB (%.2f GiB)\nStart: %lld MiB\nEnd: %lld MiB\n\n"
            "Are you sure you want to continue?",
            fs_display, format_desc, sector_sel_str ? sector_sel_str : "default", cluster_display, target_size_mib,
            (double)target_size_mib / 1024.0, actual_start, actual_end
        );

        GtkWidget *warn = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL, "%s", warn_text);
        gint warn_response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);
        g_free(warn_text);
        g_free(format_desc);
        g_free(cluster_display);

        if (warn_response == GTK_RESPONSE_OK) {
            gchar *full_cmd = g_strdup_printf(
                "sudo %s && "
                "(sudo udevadm settle && sudo partprobe %s || sudo blockdev --rereadpt %s) && "
                "sleep 2 && "
                "NEW_PART=$(%s) && "
                "if [ -z \"$NEW_PART\" ] || [ ! -b \"$NEW_PART\" ]; then echo 'Error: Partition device not found'; exit 1; fi && "
                "echo \"Partition created: $NEW_PART\" && "
                "%s && "
                "sudo udevadm settle && "
                "echo \"Filesystem %s created successfully on $NEW_PART\"",
                mkpart_cmd, disk_path, disk_path, find_part_cmd, mkfs_cmd, fs_display);
            
            GtkWidget *disk_areas_window = g_object_get_data(G_OBJECT(button), "disk_areas_window");
            GtkTreeView *main_tree_view = g_object_get_data(G_OBJECT(button), "main_tree_view");
            run_command_in_terminal(main_tree_view, full_cmd);

g_free(full_cmd);

            if (disk_areas_window && GTK_IS_WINDOW(disk_areas_window)) {
                gtk_widget_destroy(disk_areas_window);
            }
            
            show_disk_list(NULL, main_tree_view);

            if (g_strcmp0(fs_type, "ext2") == 0 || 
                g_strcmp0(fs_type, "ext3") == 0 || 
                g_strcmp0(fs_type, "ntfs") == 0) {
                g_timeout_add(13500, refresh_disk_list_delayed, g_object_ref(main_tree_view));
            } else {
                g_timeout_add(3500, refresh_disk_list_delayed, g_object_ref(main_tree_view));
            }
            
        }

        g_free(find_part_cmd);
        g_free(mkpart_cmd);
        g_free(mkfs_cmd);
    }

    gtk_widget_destroy(dialog);
    g_free(start); g_free(end); if (type) g_free(type);
    g_free(disk_path);
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

typedef struct {
    GtkWidget *entry_bytes;
    GtkWidget *entry_mib;
    GtkWidget *entry_gib;
    GtkWidget *label_info;
    gboolean updating;
} BenchmarkSizeWidgets;

static void on_benchmark_bytes_changed(GtkEditable *editable, gpointer user_data) {
    BenchmarkSizeWidgets *widgets = (BenchmarkSizeWidgets*)user_data;
    if (widgets->updating) return;

    widgets->updating = TRUE;
    const gchar *bytes_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_bytes));
    long long bytes = atoll(bytes_text);
    
    long long mib = bytes / (1024LL * 1024LL);
    long long gib = bytes / (1024LL * 1024LL * 1024LL);
    
    char buf[64];
    g_snprintf(buf, sizeof(buf), "%lld", mib);
    gtk_entry_set_text(GTK_ENTRY(widgets->entry_mib), buf);
    g_snprintf(buf, sizeof(buf), "%lld", gib);
    gtk_entry_set_text(GTK_ENTRY(widgets->entry_gib), buf);
    
    g_snprintf(buf, sizeof(buf), "Total: %lld bytes (%.2f GiB)", bytes, (double)bytes / (1024*1024*1024));
    gtk_label_set_text(GTK_LABEL(widgets->label_info), buf);
    
    widgets->updating = FALSE;
}

static void on_benchmark_mib_changed(GtkEditable *editable, gpointer user_data) {
    BenchmarkSizeWidgets *widgets = (BenchmarkSizeWidgets*)user_data;
    if (widgets->updating) return;

    widgets->updating = TRUE;
    const gchar *mib_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_mib));
    long long mib = atoll(mib_text);
    
    long long bytes = mib * 1024LL * 1024LL;
    long long gib = mib / 1024LL;
    
    char buf[64];
    g_snprintf(buf, sizeof(buf), "%lld", bytes);
    gtk_entry_set_text(GTK_ENTRY(widgets->entry_bytes), buf);
    g_snprintf(buf, sizeof(buf), "%lld", gib);
    gtk_entry_set_text(GTK_ENTRY(widgets->entry_gib), buf);
    
    g_snprintf(buf, sizeof(buf), "Total: %lld MiB (%.2f GiB)", mib, (double)mib / 1024);
    gtk_label_set_text(GTK_LABEL(widgets->label_info), buf);
    
    widgets->updating = FALSE;
}

static void on_benchmark_gib_changed(GtkEditable *editable, gpointer user_data) {
    BenchmarkSizeWidgets *widgets = (BenchmarkSizeWidgets*)user_data;
    if (widgets->updating) return;

    widgets->updating = TRUE;
    const gchar *gib_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry_gib));
    long long gib = atoll(gib_text);
    
    long long bytes = gib * 1024LL * 1024LL * 1024LL;
    long long mib = gib * 1024LL;
    
    char buf[64];
    g_snprintf(buf, sizeof(buf), "%lld", bytes);
    gtk_entry_set_text(GTK_ENTRY(widgets->entry_bytes), buf);
    g_snprintf(buf, sizeof(buf), "%lld", mib);
    gtk_entry_set_text(GTK_ENTRY(widgets->entry_mib), buf);
    
    g_snprintf(buf, sizeof(buf), "Total: %lld GiB (%.2f GB)", gib, (double)gib);
    gtk_label_set_text(GTK_LABEL(widgets->label_info), buf);
    
    widgets->updating = FALSE;
}

void on_disk_read_benchmark_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        GtkWidget *warn = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "ERROR: Select valid partition!");
        gtk_dialog_run(GTK_DIALOG(warn)); gtk_widget_destroy(warn);
        return;
    }

    gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);
    if (!partition_name || strlen(partition_name) == 0) {
        GtkWidget *warn = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "ERROR: Select valid partition!");
        gtk_dialog_run(GTK_DIALOG(warn)); gtk_widget_destroy(warn);
        g_free(partition_name);
        return;
    }

    gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

    GtkWidget *size_dialog = gtk_dialog_new_with_buttons(
        "Disk Read Speed Test - Choose Size",
        NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Start Test", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(size_dialog), 500, 350);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(size_dialog));

    GtkWidget *info_label = gtk_label_new(NULL);
    gchar *info_text = g_strdup_printf(
        "Device: %s\n"
        "Operation: Sequential read (safe)\n"
        "Default test size: 8 GiB\n\n"
        "Enter test size below:",
        device_path
    );
    gtk_label_set_text(GTK_LABEL(info_label), info_text);
    gtk_box_pack_start(GTK_BOX(content_area), info_label, FALSE, FALSE, 5);
    g_free(info_text);

    GtkWidget *entry_bytes = gtk_entry_new();
    GtkWidget *entry_mib = gtk_entry_new();
    GtkWidget *entry_gib = gtk_entry_new();
    GtkWidget *label_info = gtk_label_new("");

    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in Bytes:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_bytes, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in MiB:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_mib, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in GiB:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_gib, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 5);

    BenchmarkSizeWidgets *widgets = g_new0(BenchmarkSizeWidgets, 1);
    widgets->entry_bytes = entry_bytes;
    widgets->entry_mib = entry_mib;
    widgets->entry_gib = entry_gib;
    widgets->label_info = label_info;

    g_signal_connect(entry_bytes, "changed", G_CALLBACK(on_benchmark_bytes_changed), widgets);
    g_signal_connect(entry_mib, "changed", G_CALLBACK(on_benchmark_mib_changed), widgets);
    g_signal_connect(entry_gib, "changed", G_CALLBACK(on_benchmark_gib_changed), widgets);

    gtk_entry_set_text(GTK_ENTRY(entry_gib), "8");
    on_benchmark_gib_changed(GTK_EDITABLE(entry_gib), widgets);

    gtk_widget_show_all(size_dialog);
    gint size_response = gtk_dialog_run(GTK_DIALOG(size_dialog));

    long long test_size_mib = 0;
    if (size_response == GTK_RESPONSE_ACCEPT) {
        const gchar *mib_text = gtk_entry_get_text(GTK_ENTRY(entry_mib));
        test_size_mib = atoll(mib_text);
        if (test_size_mib <= 0) test_size_mib = 8192;
    }

    g_free(widgets);
    gtk_widget_destroy(size_dialog);

    if (size_response != GTK_RESPONSE_ACCEPT) {
        g_free(device_path);
        g_free(partition_name);
        return;
    }

    long long test_size_gib_display = test_size_mib / 1024;
    gchar *confirm_text = g_strdup_printf(
        "Disk Read Benchmark (safe)\n\n"
        "Device: %s\n"
        "Test size: %lld GiB (%lld MiB)\n"
        "Operation: Sequential read\n\n"
        "This test only reads data – safe operation.",
        device_path, test_size_gib_display, test_size_mib
    );

    GtkWidget *confirm_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "%s", confirm_text);
    gint response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    g_free(confirm_text);

    if (response == GTK_RESPONSE_YES) {
        gchar *cmd = g_strdup_printf(
            "sudo sh -c \"echo 3 > /proc/sys/vm/drop_caches\" && "
            "dd if=%s of=/dev/null bs=1M count=%lld status=progress iflag=direct",
            device_path, test_size_mib
        );
        run_command_in_terminal(tree_view, cmd);
        g_free(cmd);
    }

    g_free(device_path);
    g_free(partition_name);
}

static gboolean check_tmp_space_for_size(long long required_gib) {
    FILE *space_fp = popen("df --output=avail /tmp | tail -1 | tr -d ' '", "r");
    gchar space_buf[16] = {0};
    long long free_kb = 0;

    if (space_fp) {
        if (fgets(space_buf, sizeof(space_buf), space_fp)) {
            g_strstrip(space_buf);
            free_kb = atoll(space_buf);
        }
        pclose(space_fp);
    }

    long long required_kb = required_gib * 1024LL * 1024LL;
    return free_kb >= required_kb;
}

void on_disk_file_write_benchmark_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        return;
    }

    gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);

    if (!partition_name || strlen(partition_name) == 0) {
        GtkWidget *warn = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "ERROR: Select valid partition!");
        gtk_dialog_run(GTK_DIALOG(warn)); gtk_widget_destroy(warn);
        g_free(partition_name);
        return;
    }

    gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

    GtkWidget *size_dialog = gtk_dialog_new_with_buttons(
        "File Write Speed Test - Choose Size",  // ✅ Оновлено назву
        NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Start Test", GTK_RESPONSE_ACCEPT,
        NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(size_dialog), 500, 350);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(size_dialog));

    GtkWidget *info_label = gtk_label_new(NULL);
    gchar *info_text = g_strdup_printf(
        "Device: %s\n"
        "Operation: Sequential file write (safe)\n"
        "Default test size: 8 GiB\n\n"
        "Enter test size below:\n"
        "(File will be created in home folder and auto-deleted)",  // ✅ Змінено /tmp → home
        device_path
    );
    gtk_label_set_text(GTK_LABEL(info_label), info_text);
    gtk_box_pack_start(GTK_BOX(content_area), info_label, FALSE, FALSE, 5);
    g_free(info_text);

    GtkWidget *entry_bytes = gtk_entry_new();
    GtkWidget *entry_mib = gtk_entry_new();
    GtkWidget *entry_gib = gtk_entry_new();
    GtkWidget *label_info = gtk_label_new("");

    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in Bytes:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_bytes, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in MiB:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_mib, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in GiB:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), entry_gib, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 5);

    BenchmarkSizeWidgets *widgets = g_new0(BenchmarkSizeWidgets, 1);
    widgets->entry_bytes = entry_bytes;
    widgets->entry_mib = entry_mib;
    widgets->entry_gib = entry_gib;
    widgets->label_info = label_info;

    g_signal_connect(entry_bytes, "changed", G_CALLBACK(on_benchmark_bytes_changed), widgets);
    g_signal_connect(entry_mib, "changed", G_CALLBACK(on_benchmark_mib_changed), widgets);
    g_signal_connect(entry_gib, "changed", G_CALLBACK(on_benchmark_gib_changed), widgets);

    gtk_entry_set_text(GTK_ENTRY(entry_gib), "8");
    on_benchmark_gib_changed(GTK_EDITABLE(entry_gib), widgets);

    gtk_widget_show_all(size_dialog);
    gint size_response = gtk_dialog_run(GTK_DIALOG(size_dialog));

    long long test_size_mib = 0;
    if (size_response == GTK_RESPONSE_ACCEPT) {
        const gchar *mib_text = gtk_entry_get_text(GTK_ENTRY(entry_mib));
        test_size_mib = atoll(mib_text);
        if (test_size_mib <= 0) test_size_mib = 8192;
    }

    g_free(widgets);
    gtk_widget_destroy(size_dialog);

    if (size_response != GTK_RESPONSE_ACCEPT) {
        g_free(device_path);
        g_free(partition_name);
        return;
    }

    FILE *space_fp = popen("df --output=avail $HOME | tail -1 | tr -d ' '", "r");
    gchar space_buf[16] = {0};
    long long free_kb = 0;
    if (space_fp) {
        if (fgets(space_buf, sizeof(space_buf), space_fp)) {
            g_strstrip(space_buf);
            free_kb = atoll(space_buf);
        }
        pclose(space_fp);
    }
    long long required_kb = (test_size_mib / 1024) * 1024LL * 1024LL;
    if (free_kb < required_kb) {
        GtkWidget *warn = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
            "ERROR: Insufficient space in home folder!\n\n"
            "Required: %.1f GiB\n"
            "Please free up space or use Raw Write test.",
            (double)test_size_mib / 1024
        );
        gtk_dialog_run(GTK_DIALOG(warn)); gtk_widget_destroy(warn);
        g_free(device_path);
        g_free(partition_name);
        return;
    }

    gchar *test_file = g_strdup_printf("$HOME/benchmark-%s.dat", partition_name);
    
    long long test_size_gib_display = test_size_mib / 1024;
    gchar *size_display = g_strdup_printf("%lld MiB (%.2f GiB)", test_size_mib, (double)test_size_gib_display);
    gchar *warn_text = g_strdup_printf(
        "Disk File Write Benchmark (safe)\n\n"
        "Device: %s\n"
        "Test size: %s\n"
        "Test file: %s\n\n"
        "This test creates temporary file in home folder\n"
        "File will be automatically deleted after test!\n"
        "It's a safe operation.",
        device_path, size_display, test_file
    );

    GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO, "%s", warn_text);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response == GTK_RESPONSE_YES) {
        gchar *cmd = g_strdup_printf(
            "sudo sh -c \"echo 3 > /proc/sys/vm/drop_caches\" && "
            "sync && "
            "dd if=/dev/zero of='%s' bs=1M count=%lld status=progress oflag=direct && "
            "sync && rm -f '%s'",
            test_file, test_size_mib, test_file
        );
        run_command_in_terminal(tree_view, cmd);
        g_free(cmd);
    }

    g_free(test_file);
    g_free(size_display);
    g_free(warn_text);
    g_free(device_path);
    g_free(partition_name);
}

void on_disk_raw_write_benchmark_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, -1);

        if (!partition_name || strlen(partition_name) == 0) {
            GtkWidget *warn = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "ERROR: Select valid partition!");
            gtk_dialog_run(GTK_DIALOG(warn)); gtk_widget_destroy(warn);
            g_free(partition_name);
            return;
        }

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

        GtkWidget *size_dialog = gtk_dialog_new_with_buttons(
            "Raw Write Speed Test - DANGEROUS",
            NULL,
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Continue", GTK_RESPONSE_ACCEPT,
            NULL
        );
        gtk_window_set_default_size(GTK_WINDOW(size_dialog), 500, 350);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(size_dialog));

        GtkWidget *info_label = gtk_label_new(NULL);
        gchar *info_text = g_strdup_printf(
            "WARNING: Raw Device Write (DESTRUCTIVE)\n\n"
            "Device: %s\n"
            "Default test size: 8 GiB\n\n"
            "Enter test size below:\n"
            "DESTROYS ALL DATA in first N GiB of device!\n\n"
            "Continue only if you understand the risk:",
            device_path
        );
        gtk_label_set_text(GTK_LABEL(info_label), info_text);
        gtk_box_pack_start(GTK_BOX(content_area), info_label, FALSE, FALSE, 5);
        g_free(info_text);

        GtkWidget *entry_bytes = gtk_entry_new();
        GtkWidget *entry_mib = gtk_entry_new();
        GtkWidget *entry_gib = gtk_entry_new();
        GtkWidget *label_info = gtk_label_new("");

        gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in Bytes:"), FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(content_area), entry_bytes, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in MiB:"), FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(content_area), entry_mib, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Size in GiB:"), FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(content_area), entry_gib, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(content_area), label_info, FALSE, FALSE, 5);

        BenchmarkSizeWidgets *widgets = g_new0(BenchmarkSizeWidgets, 1);
        widgets->entry_bytes = entry_bytes;
        widgets->entry_mib = entry_mib;
        widgets->entry_gib = entry_gib;
        widgets->label_info = label_info;

        g_signal_connect(entry_bytes, "changed", G_CALLBACK(on_benchmark_bytes_changed), widgets);
        g_signal_connect(entry_mib, "changed", G_CALLBACK(on_benchmark_mib_changed), widgets);
        g_signal_connect(entry_gib, "changed", G_CALLBACK(on_benchmark_gib_changed), widgets);

        gtk_entry_set_text(GTK_ENTRY(entry_gib), "8");
        on_benchmark_gib_changed(GTK_EDITABLE(entry_gib), widgets);

        gtk_widget_show_all(size_dialog);
        gint size_response = gtk_dialog_run(GTK_DIALOG(size_dialog));

        long long test_size_mib = 0;
        if (size_response == GTK_RESPONSE_ACCEPT) {
            const gchar *mib_text = gtk_entry_get_text(GTK_ENTRY(entry_mib));
            test_size_mib = atoll(mib_text);
            if (test_size_mib <= 0) test_size_mib = 8192;
        }

        g_free(widgets);
        gtk_widget_destroy(size_dialog);

        if (size_response != GTK_RESPONSE_ACCEPT) {
            g_free(device_path);
            g_free(partition_name);
            return;
        }

        long long test_size_gib_display = test_size_mib / 1024;
        gchar *size_display = g_strdup_printf("%lld MiB (%.2f GiB)", test_size_mib, (double)test_size_gib_display);
        gchar *warn_text = g_strdup_printf(
            "WARNING: Raw Device Write (destructive)\n\n"
            "Device: %s\n"
            "Test size: %s\n"
            "Operation: Direct write to raw device\n\n"
            "THIS WILL DESTROY ALL DATA in first %lld GiB of selected device!\n\n"
            "Are you sure you want to continue?",
            device_path, size_display, test_size_gib_display
        );

        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO, "%s", warn_text);
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (response == GTK_RESPONSE_YES) {
            gchar *cmd = g_strdup_printf(
                "sudo sh -c \"echo 3 > /proc/sys/vm/drop_caches\" && "
                "sync && "
                "dd if=/dev/zero of=%s bs=1M count=%lld status=progress oflag=direct && "
                "sync",
                device_path, test_size_mib
            );
            run_command_in_terminal(tree_view, cmd);
            g_free(cmd);
        }

        g_free(size_display);
        g_free(warn_text);
        g_free(device_path);
        g_free(partition_name);
    }
}

void on_auto_fsck_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;
    gchar *filesystem = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 
                          COL_NAME, &partition_name, 
                          COL_FSTYPE, &filesystem, 
                          -1);

        if (!filesystem) {
            GtkWidget *warn = gtk_message_dialog_new(
                NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "ERROR: Could not determine filesystem type for partition %s", 
                partition_name
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);
            g_free(partition_name);
            return;
        }

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);
        gchar *cmd = NULL;
        gchar *fs_name = NULL;

        if (g_strcmp0(filesystem, "ext4") == 0 || 
            g_strcmp0(filesystem, "ext3") == 0 || 
            g_strcmp0(filesystem, "ext2") == 0) {
            cmd = g_strdup_printf("e2fsck -f -y -v %s", device_path);
            fs_name = g_strdup("EXT2/3/4");
        }
        else if (g_strcmp0(filesystem, "vfat") == 0 || 
                 g_strcmp0(filesystem, "fat32") == 0) {
            cmd = g_strdup_printf("dosfsck -a -v %s", device_path);
            fs_name = g_strdup("FAT32");
        }
        else if (g_strcmp0(filesystem, "ntfs") == 0) {
            cmd = g_strdup_printf("ntfsfix %s", device_path);
            fs_name = g_strdup("NTFS");
        }
        else if (g_strcmp0(filesystem, "xfs") == 0) {
            cmd = g_strdup_printf("xfs_repair %s", device_path);
            fs_name = g_strdup("XFS");
        }
        else if (g_strcmp0(filesystem, "btrfs") == 0) {
            cmd = g_strdup_printf("btrfs check --repair %s", device_path);
            fs_name = g_strdup("Btrfs");
        }
        else if (g_strcmp0(filesystem, "f2fs") == 0) {
            cmd = g_strdup_printf("fsck.f2fs -f %s", device_path);
            fs_name = g_strdup("F2FS");
        }
        else {
            GtkWidget *warn = gtk_message_dialog_new(
                NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK,
                "Filesystem '%s' is not supported for automatic repair.\n\n"
                "Supported filesystems:\n"
                "• ext2/ext3/ext4\n"
                "• FAT32 (vfat)\n"
                "• NTFS\n"
                "• XFS\n"
                "• Btrfs\n"
                "• F2FS",
                filesystem
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);
            g_free(device_path);
            g_free(partition_name);
            g_free(filesystem);
            return;
        }

        gchar *confirm_msg = g_strdup_printf(
            "Auto-detected filesystem: %s (%s)\n"
            "Partition: %s\n\n"
            "Command to execute: %s\n\n"
            "Do you want to proceed with filesystem check?",
            fs_name, filesystem, device_path, cmd
        );

        GtkWidget *confirm = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
            "%s", confirm_msg
        );
        
        gint response = gtk_dialog_run(GTK_DIALOG(confirm));
        gtk_widget_destroy(confirm);
        g_free(confirm_msg);

        if (response == GTK_RESPONSE_YES) {
            run_command_in_terminal(tree_view, cmd);
        }

        g_free(cmd);
        g_free(fs_name);
        g_free(device_path);
        g_free(partition_name);
        g_free(filesystem);
    }
}

void on_e2fsck_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;
    gchar *filesystem = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           COL_NAME, &partition_name,
                           COL_FSTYPE, &filesystem,
                           -1);

        if (!filesystem || g_strcmp0(filesystem, "ext4") != 0 &&
                          g_strcmp0(filesystem, "ext3") != 0 &&
                          g_strcmp0(filesystem, "ext2") != 0) {

            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "ERROR: e2fsck can be used **only** on ext2/3/4 partitions!"
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            g_free(partition_name);
            g_free(filesystem);
            return;
        }

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);
        gchar *cmd = g_strdup_printf("e2fsck -f -y -v %s", device_path);

        run_command_in_terminal(tree_view, cmd);

        g_free(cmd);
        g_free(device_path);
        g_free(partition_name);
        g_free(filesystem);
    }
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
                run_command_in_terminal(tree_view, repair_cmd);
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

void on_fat32fix_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;
    gchar *filesystem = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           COL_NAME, &partition_name,
                           COL_FSTYPE, &filesystem,
                           -1);

        if (!filesystem || g_strcmp0(filesystem, "vfat") != 0) {
            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "ERROR: dosfsck can be used **only** on FAT32 (vfat) partitions!"
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            g_free(partition_name);
            g_free(filesystem);
            return;
        }

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);
        gchar *cmd = g_strdup_printf("dosfsck -a -v %s", device_path);

        run_command_in_terminal(tree_view, cmd);

        g_free(cmd);
        g_free(device_path);
        g_free(partition_name);
        g_free(filesystem);
    }
}

void on_ntfsfix_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL;
    gchar *filesystem = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter,
                           COL_NAME, &partition_name,
                           COL_FSTYPE, &filesystem,
                           -1);

        if (!filesystem || g_strcmp0(filesystem, "ntfs") != 0) {
            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "ERROR: ntfsfix can be used **only** on NTFS partitions!"
            );
            gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            g_free(partition_name);
            g_free(filesystem);
            return;
        }

        gchar *device_path = g_strdup_printf("/dev/%s", partition_name);
        gchar *cmd = g_strdup_printf("ntfsfix %s", device_path);

        run_command_in_terminal(tree_view, cmd);

        g_free(cmd);
        g_free(device_path);
        g_free(partition_name);
        g_free(filesystem);
    }
}

void on_ntfsresize_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command_in_terminal(GTK_TREE_VIEW(user_data), "ntfsresize -P -i -f -v %s");
}

void on_diskscan_activate(GtkWidget *menuitem, gpointer user_data) {
    run_command_in_terminal(GTK_TREE_VIEW(user_data), "diskscan %s");
}

static void on_mount_child_exited(VteTerminal *terminal, gint status, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(terminal));
    if (GTK_IS_WINDOW(window)) {
        gtk_window_set_title(GTK_WINDOW(window), "Command Finished");
    }
    
    refresh_disk_list_delayed(tree_view);
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
        
        gchar *mkdir_command = g_strdup_printf("sudo mkdir -p %s", quoted_mount_point);
        system(mkdir_command);
        g_free(mkdir_command);
        
        gchar *command = g_strdup_printf(
            "if [ -b %s ]; then "
            "  echo 'Mounting %s to %s...'; "
            "  timeout 15 sudo mount %s %s && "
            "  echo 'Mounted successfully' || "
            "  (sleep 2 && timeout 15 sudo mount %s %s && "
            "   echo 'Mounted on second attempt' || "
            "   echo 'ERROR: Mount failed after 2 attempts'); "
            "else "
            "  echo 'ERROR: Device %s not found'; "
            "fi",
            quoted_device_path,
            device_path, 
            mount_point,
            quoted_device_path, quoted_mount_point,
            quoted_device_path, quoted_mount_point,
            device_path
        );
        
        run_command_in_terminal(tree_view, command);
        
        g_free(command);
        g_free(quoted_device_path);
        g_free(quoted_mount_point);
        g_free(device_path);
        g_free(mount_point);
        g_free(disk_name);
    }
}

void on_umount_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *quoted_device_path = g_shell_quote(device_path);
        
        gchar *command = g_strdup_printf(
            "echo 'Unmounting %s...'; "
            "if mount | grep -q '^%s '; then "
                "timeout 15 sudo umount %s && "
                "echo 'Unmounted successfully' || "
                "(sleep 2 && timeout 15 sudo umount %s && "
                "echo 'Unmounted on second attempt' || "
                "echo 'ERROR: Unmount failed after 2 attempts'); "
            "else "
                "echo 'Device %s is not mounted - unmounting not needed'; "
            "fi",
            device_path,
            device_path,
            quoted_device_path,
            quoted_device_path,
            device_path
        );
        
        run_command_in_terminal(tree_view, command);
        
        g_free(command);
        g_free(quoted_device_path);
        g_free(device_path);
        g_free(disk_name);
    }
}

void on_umount_l_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *quoted_device_path = g_shell_quote(device_path);
        
        gchar *command = g_strdup_printf(
            "echo 'Lazy unmounting %s...'; "
            "if mount | grep -q '^%s '; then "
                "timeout 15 sudo umount -l %s && "
                "echo 'Lazy unmount successful' || "
                "(sleep 2 && timeout 15 sudo umount -l %s && "
                "echo 'Lazy unmount successful on second attempt' || "
                "echo 'ERROR: Lazy unmount failed'); "
            "else "
                "echo 'Device %s is not mounted - lazy unmounting not needed'; "
            "fi",
            device_path,
            device_path,
            quoted_device_path,
            quoted_device_path,
            device_path
        );
        
        run_command_in_terminal(tree_view, command);
        
        g_free(command);
        g_free(quoted_device_path);
        g_free(device_path);
        g_free(disk_name);
    }
}

void on_umount_f_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *disk_name = NULL;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &disk_name, -1);

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        gchar *quoted_device_path = g_shell_quote(device_path);
        
        gchar *command = g_strdup_printf(
            "echo 'Force unmounting %s...'; "
            "if mount | grep -q '^%s '; then "
                "timeout 15 sudo umount -f %s && "
                "echo 'Forced unmount successful' || "
                "(sleep 2 && timeout 15 sudo umount -f %s && "
                "echo 'Forced unmount successful on second attempt' || "
                "echo 'ERROR: Forced unmount failed'); "
            "else "
                "echo 'Device %s is not mounted - force unmounting not needed'; "
            "fi",
            device_path,
            device_path,
            quoted_device_path,
            quoted_device_path,
            device_path
        );
        
        run_command_in_terminal(tree_view, command);
        
        g_free(command);
        g_free(quoted_device_path);
        g_free(device_path);
        g_free(disk_name);
    }
}

void on_rename_partition_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *fstype = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &partition_name, COL_FSTYPE, &fstype, -1);

        gchar *current_label = NULL;
        gchar *label_cmd = NULL;
        
        if (g_strcmp0(fstype, "ext4") == 0 || g_strcmp0(fstype, "ext3") == 0 || g_strcmp0(fstype, "ext2") == 0) {
            label_cmd = g_strdup_printf("blkid -s LABEL -o value /dev/%s 2>/dev/null", partition_name);
        } else if (g_strcmp0(fstype, "ntfs") == 0) {
            label_cmd = g_strdup_printf("blkid -s LABEL -o value /dev/%s 2>/dev/null", partition_name);
        } else if (g_strcmp0(fstype, "exfat") == 0) {
            label_cmd = g_strdup_printf("blkid -s LABEL -o value /dev/%s 2>/dev/null", partition_name);
        } else if (g_strcmp0(fstype, "vfat") == 0 || g_strcmp0(fstype, "fat32") == 0 || g_strcmp0(fstype, "fat") == 0) {
            label_cmd = g_strdup_printf("blkid -s LABEL -o value /dev/%s 2>/dev/null", partition_name);
        } else if (g_strcmp0(fstype, "xfs") == 0) {
            label_cmd = g_strdup_printf("xfs_admin -l /dev/%s 2>/dev/null | cut -d'\"' -f2", partition_name);
        } else if (g_strcmp0(fstype, "btrfs") == 0) {
            label_cmd = g_strdup_printf("btrfs filesystem label /dev/%s 2>/dev/null || echo ''", partition_name);
        } else {
            label_cmd = g_strdup_printf("blkid -s LABEL -o value /dev/%s 2>/dev/null", partition_name);
        }
        
        if (label_cmd) {
            FILE *fp = popen(label_cmd, "r");
            if (fp) {
                char label_buf[256] = {0};
                if (fgets(label_buf, sizeof(label_buf), fp)) {
                    g_strstrip(label_buf);
                    if (strlen(label_buf) > 0) {
                        current_label = g_strdup(label_buf);
                    }
                }
                pclose(fp);
            }
            g_free(label_cmd);
        }

        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Rename Partition",
            NULL,
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Rename", GTK_RESPONSE_ACCEPT,
            NULL
        );
        
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 140);
        
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        
        gchar *info_text;
        if (current_label && strlen(current_label) > 0) {
            info_text = g_strdup_printf("Partition: %s (%s)\nCurrent label: %s", partition_name, fstype, current_label);
        } else {
            info_text = g_strdup_printf("Partition: %s (%s)\nCurrent label: (empty)", partition_name, fstype);
        }
        GtkWidget *info_label = gtk_label_new(info_text);
        gtk_label_set_xalign(GTK_LABEL(info_label), 0.0);
        gtk_box_pack_start(GTK_BOX(content_area), info_label, FALSE, FALSE, 10);
        g_free(info_text);
        
        GtkWidget *entry = gtk_entry_new();
        gtk_entry_set_max_length(GTK_ENTRY(entry), 16);
        gtk_entry_set_width_chars(GTK_ENTRY(entry), 24);
        
        if (current_label && strlen(current_label) > 0) {
            gtk_entry_set_text(GTK_ENTRY(entry), current_label);
            gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
        }
        
        gtk_box_pack_start(GTK_BOX(content_area), entry, FALSE, FALSE, 5);
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
                cmd = g_strdup_printf(
                    "sudo e2label %%s '%s' && echo 'Label changed (ext2/3/4)' || echo 'ERROR: Failed to change ext label'",
                    new_label
                );
            } else if (g_strcmp0(fstype, "ntfs") == 0) {
                cmd = g_strdup_printf(
                    "sudo ntfslabel --force %%s '%s' >/dev/null 2>&1 && echo 'Label changed (NTFS)' || echo 'ERROR: Failed to change NTFS label'",
                    new_label
                );
            } else if (g_strcmp0(fstype, "exfat") == 0) {
                cmd = g_strdup_printf(
                    "sudo exfatlabel %%s '%s' && echo 'Label changed (exFAT)' || echo 'ERROR: Failed to change exFAT label'",
                    new_label
                );
            } else if (g_strcmp0(fstype, "vfat") == 0 || g_strcmp0(fstype, "fat32") == 0 || g_strcmp0(fstype, "fat") == 0) {
                cmd = g_strdup_printf(
                    "{ sudo dosfslabel %%s '%s' 2>&1 || sudo fatlabel %%s '%s' 2>&1; } | "
                    "grep -v 'differences\\|Differences\\|offset\\|Not automatically\\|backup' && "
                    "echo 'Label changed (FAT)' || echo 'Label changed (FAT)'",
                    new_label, new_label
                );
            } else if (g_strcmp0(fstype, "xfs") == 0) {
                cmd = g_strdup_printf(
                    "sudo xfs_admin -L '%s' %%s && echo 'Label changed (XFS)' || echo 'ERROR: Failed to change XFS label'",
                    new_label
                );
            } else if (g_strcmp0(fstype, "btrfs") == 0) {
                cmd = g_strdup_printf(
                    "sudo btrfs filesystem label %%s '%s' && echo 'Label changed (Btrfs)' || echo 'ERROR: Failed to change Btrfs label'",
                    new_label
                );
            } else {
                cmd = g_strdup_printf("echo 'Unsupported or unknown filesystem: %s'", fstype ? fstype : "unknown");
            }

            run_command_in_terminal(tree_view, cmd);
            g_free(cmd);
            g_free(new_label);
        }

        if (current_label) g_free(current_label);
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
                    "This function is intended only for entire disks (e.g., /dev/sdb), not for partitions (e.g., /dev/sdb1). Please select a whole disk (not a partition)."
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
        gtk_window_set_default_size(GTK_WINDOW(dialog), 350, 70);

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
                    "(sudo parted -s %s mklabel \"%s\" || %s) && "
                    "sudo partprobe %s || sudo blockdev --rereadpt %s && "
                    "sudo parted -s %s print | grep -q '^Number  Start' && "
                    "[ -z \"$(sudo parted -s %s print | grep '^ [0-9]')\" ] && "
                    "echo 'Partition table (%s) created successfully.' || "
                    "echo 'Failed: partition table (%s) not created.'",
                    quoted_device, quoted_device, quoted_device, table, fdisk_cmd, 
                    quoted_device, quoted_device, quoted_device, quoted_device, table, table
                );
                run_command_in_terminal(tree_view, command);
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
    gchar *type = NULL;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;

    gtk_tree_model_get(model, &iter,
        COL_NAME, &partition_name,
        COL_MOUNTPOINT, &mountpoint,
        COL_TYPE, &type,
        -1);

    if (g_strcmp0(type, "disk") == 0) {
        GtkWidget *warn_dialog = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
            "WARNING: You are about to create a filesystem directly on the disk (%s) without creating a partition table!\n\n"
            "This is NOT recommended. Use 'Show Filesystems and Free Space' to create a partition with a filesystem instead.\n\n"
            "Do you really want to continue?", partition_name
        );
        gint warn_resp = gtk_dialog_run(GTK_DIALOG(warn_dialog));
        gtk_widget_destroy(warn_dialog);
        if (warn_resp != GTK_RESPONSE_YES) {
            g_free(partition_name);
            g_free(type);
            if (mountpoint) g_free(mountpoint);
            return;
        }
    }

    gchar *device_path = g_strdup_printf("/dev/%s", partition_name);

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Format Filesystem", NULL, GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL, "_Create", GTK_RESPONSE_ACCEPT, NULL
    );
    gtk_window_set_default_size(GTK_WINDOW(dialog), 420, 180);
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext4");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext3");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ext2");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ntfs (quick format)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "ntfs (full zeroing)");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "exfat");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), "fat32");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    GtkWidget *cluster_label = gtk_label_new("Cluster / Block size:");
    GtkWidget *cluster_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cluster_combo), "default");
    const int cluster_options[] = {512,1024,2048,4096,8192,16384};
    for (int i = 0; i < 6; i++) {
        gchar tmp[16];
        g_snprintf(tmp, sizeof(tmp), "%d", cluster_options[i]);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(cluster_combo), tmp);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(cluster_combo), 0);

    GtkWidget *sector_label = gtk_label_new("Logical sector size (bytes):");
    GtkWidget *sector_combo = gtk_combo_box_text_new();
    const int sector_options[] = {512,1024,2048,4096,8192,16384};
    int default_sector = 512;
    for (int i = 0; i < 6; i++) {
        gchar tmp[32];
        g_snprintf(tmp, sizeof(tmp), "%d%s", sector_options[i],
                   sector_options[i]==default_sector ? " (default)" : "");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(sector_combo), tmp);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(sector_combo), 0);

    gtk_box_pack_start(GTK_BOX(content_area), gtk_label_new("Select filesystem type:"), FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), combo, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), sector_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), sector_combo, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), cluster_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(content_area), cluster_combo, FALSE, FALSE, 2);

    GtkWidget *spacer = gtk_label_new("");
    gtk_widget_set_size_request(spacer, -1, 15);
    gtk_box_pack_start(GTK_BOX(content_area), spacer, FALSE, FALSE, 0);

    gtk_widget_show_all(dialog);

    ClusterUpdateData cud;
    cud.cluster_label = cluster_label;
    cud.cluster_combo = cluster_combo;
    g_signal_connect(combo, "changed", G_CALLBACK(update_cluster_options_cb), &cud);
    update_cluster_options_cb(GTK_COMBO_BOX_TEXT(combo), &cud);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        const gchar *fs_display = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        const gchar *cluster_sel = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(cluster_combo));
        const gchar *sector_sel_str = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(sector_combo));

        gboolean is_ntfs_quick = g_strcmp0(fs_display, "ntfs (quick format)") == 0;
        gboolean is_ntfs_full = g_strcmp0(fs_display, "ntfs (full zeroing)") == 0;
        const gchar *fs_type = is_ntfs_quick || is_ntfs_full ? "ntfs" : fs_display;

        int chosen_sector = 512;
        if (sector_sel_str) {
            char *clean = g_strdup(sector_sel_str);
            char *p = strstr(clean, " (default)");
            if (p) *p = 0;
            chosen_sector = atoi(clean);
            g_free(clean);
        }

        int cluster_val = 0;
        if (cluster_sel && g_strcmp0(cluster_sel,"default")!=0) {
            cluster_val = atoi(cluster_sel);
        } else {
            if (g_strcmp0(fs_type,"ext4")==0 || g_strcmp0(fs_type,"ext3")==0) cluster_val = 4096;
            else if (g_strcmp0(fs_type,"ext2")==0) cluster_val = 1024;
            else if (g_strcmp0(fs_type,"exfat")==0) cluster_val = 8;
            else if (g_strcmp0(fs_type,"fat32")==0) cluster_val = 1;
        }

        gchar *mkfs_cmd = NULL;
        if (g_strcmp0(fs_type,"ext4")==0)
            mkfs_cmd = g_strdup_printf("mkfs.ext4 -F -b %d '%s'", cluster_val, device_path);
        else if (g_strcmp0(fs_type,"ext3")==0)
            mkfs_cmd = g_strdup_printf("mkfs.ext3 -F -b %d '%s'", cluster_val, device_path);
        else if (g_strcmp0(fs_type,"ext2")==0)
            mkfs_cmd = g_strdup_printf("mkfs.ext2 -F -b %d '%s'", cluster_val, device_path);
        else if (is_ntfs_quick) {
            if (cluster_val > 0)
                mkfs_cmd = g_strdup_printf("mkfs.ntfs -f -c %d -Q '%s'", cluster_val, device_path);
            else
                mkfs_cmd = g_strdup_printf("mkfs.ntfs -f -Q '%s'", device_path);
        }
        else if (is_ntfs_full) {
            if (cluster_val > 0)
                mkfs_cmd = g_strdup_printf("mkfs.ntfs -F -c %d '%s'", cluster_val, device_path);
            else
                mkfs_cmd = g_strdup_printf("mkfs.ntfs -F '%s'", device_path);
        }
        else if (g_strcmp0(fs_type,"exfat")==0)
            mkfs_cmd = g_strdup_printf("mkfs.exfat -s %d '%s'", cluster_val, device_path);
        else if (g_strcmp0(fs_type,"fat32")==0)
            mkfs_cmd = g_strdup_printf("mkfs.vfat -F 32 -s %d '%s'", cluster_val, device_path);

        GtkWidget *warn = gtk_message_dialog_new(
            NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
            "WARNING: This will destroy all data on %s!\n\nFilesystem type: %s\nCluster/Block: %d\nSector: %d\n\nAre you sure?",
            device_path, fs_display, cluster_val, chosen_sector
        );
        gint warn_response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (warn_response == GTK_RESPONSE_OK) {
            gboolean requires_reread = (g_strcmp0(fs_type,"exfat")!=0 && g_strcmp0(fs_type,"fat32")!=0);
            gchar *command = NULL;

            if (mountpoint && strlen(mountpoint)>0 && strcmp(mountpoint,"N/A")!=0 && strcmp(mountpoint,"-")!=0) {
                if (requires_reread) {
                    command = g_strdup_printf(
                        "echo 'Unmounting %s...'; umount '%s' 2>/dev/null; "
                        "echo 'Formatting %s as %s...'; sudo %s; "
                        "sudo udevadm settle && "
                        "(sudo partprobe '%s' || sudo blockdev --rereadpt '%s'); "
                        "echo 'Filesystem created and partition table re-read.'",
                        device_path, device_path, device_path, fs_display, mkfs_cmd, device_path, device_path
                    );
                } else {
                    command = g_strdup_printf(
                        "echo 'Unmounting %s...'; umount '%s' 2>/dev/null; "
                        "echo 'Formatting %s as %s...'; sudo %s; "
                        "sudo udevadm settle && "
                        "echo 'Filesystem created.'",
                        device_path, device_path, device_path, fs_display, mkfs_cmd
                    );
                }
            } else {
                if (requires_reread) {
                    command = g_strdup_printf(
                        "echo 'Formatting %s as %s...'; sudo %s; "
                        "sudo udevadm settle && "
                        "(sudo partprobe '%s' || sudo blockdev --rereadpt '%s'); "
                        "echo 'Filesystem created and partition table re-read.'",
                        device_path, fs_display, mkfs_cmd, device_path, device_path
                    );
                } else {
                    command = g_strdup_printf(
                        "echo 'Formatting %s as %s...'; sudo %s; "
                        "sudo udevadm settle && "
                        "echo 'Filesystem created.'",
                        device_path, fs_display, mkfs_cmd
                    );
                }
            }
            run_command_in_terminal(tree_view, command);
            g_free(command);
            
            show_disk_list(NULL, tree_view);
            if (g_strcmp0(fs_type, "ext2") == 0 || 
                g_strcmp0(fs_type, "ext3") == 0 || 
                is_ntfs_full) {
                g_timeout_add(12000, refresh_disk_list_delayed, g_object_ref(tree_view));
            } else {
                g_timeout_add(2000, refresh_disk_list_delayed, g_object_ref(tree_view));
            }
        }
        g_free(mkfs_cmd);
    }

    gtk_widget_destroy(dialog);
    g_free(device_path);
    g_free(partition_name);
    g_free(type);
    if (mountpoint) g_free(mountpoint);
}

typedef struct {
    GtkWidget *label_free;
    long long orig_free;
    long long orig_size;
    int sector_size;

    GtkWidget *entry_start;
    GtkWidget *entry_end;
    GtkWidget *entry_mib;

    GtkWidget *entry_bytes;
    GtkWidget *entry_gib;
    GtkWidget *entry_sectors;
    gboolean updating;
} ResizeWidgets;

static gboolean is_updating = FALSE;

static void on_resize_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
static void on_resize_dialog_destroy(GtkWidget *dialog, gpointer user_data);
static void on_entry_bytes_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_mib_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_gib_changed(GtkEditable *editable, gpointer user_data);
static void on_entry_sectors_changed(GtkEditable *editable, gpointer user_data);

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

static void on_entry_bytes_changed(GtkEditable *editable, gpointer user_data) {
    ResizeWidgets *w = (ResizeWidgets*)user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (!text || strlen(text) == 0) return;

    long long bytes = atoll(text);
    long long mib = bytes / (1024*1024);
    long long gib = bytes / (1024*1024*1024);
    long long sectors = (bytes + w->sector_size - 1) / w->sector_size;

    g_signal_handlers_block_by_func(w->entry_mib,   G_CALLBACK(on_entry_mib_changed), w);
    g_signal_handlers_block_by_func(w->entry_gib,   G_CALLBACK(on_entry_gib_changed), w);
    g_signal_handlers_block_by_func(w->entry_sectors, G_CALLBACK(on_entry_sectors_changed), w);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", mib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_mib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", gib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_gib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", sectors);
    gtk_entry_set_text(GTK_ENTRY(w->entry_sectors), tmp);

    g_signal_handlers_unblock_by_func(w->entry_mib,   G_CALLBACK(on_entry_mib_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_gib,   G_CALLBACK(on_entry_gib_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_sectors, G_CALLBACK(on_entry_sectors_changed), w);
}

static void on_entry_mib_changed(GtkEditable *editable, gpointer user_data) {
    if (is_updating) return;
    is_updating = 1;

    ResizeWidgets *w = (ResizeWidgets*)user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (!text || strlen(text) == 0) { is_updating = 0; return; }

    long long input_mib = atoll(text);
    if (input_mib < 0) input_mib = 0;

    long long bytes = input_mib * 1024LL * 1024LL;
    double gib = (double)input_mib / 1024.0;
    long long sectors = (bytes + w->sector_size - 1) / w->sector_size;

    g_signal_handlers_block_by_func(w->entry_bytes,   G_CALLBACK(on_entry_bytes_changed), w);
    g_signal_handlers_block_by_func(w->entry_gib,     G_CALLBACK(on_entry_gib_changed), w);
    g_signal_handlers_block_by_func(w->entry_sectors, G_CALLBACK(on_entry_sectors_changed), w);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", bytes);
    gtk_entry_set_text(GTK_ENTRY(w->entry_bytes), tmp);

    g_snprintf(tmp, sizeof(tmp), "%lld", input_mib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_mib), tmp);

    g_snprintf(tmp, sizeof(tmp), "%.2f", gib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_gib), tmp);

    g_snprintf(tmp, sizeof(tmp), "%lld", sectors);
    gtk_entry_set_text(GTK_ENTRY(w->entry_sectors), tmp);

    const char *start_text = gtk_entry_get_text(GTK_ENTRY(w->entry_start));
    if (start_text && strlen(start_text) > 0) {
        long long start = atoll(start_text);
        if (start >= 0 && sectors > 0) {
            long long end = start + sectors - 1;
            g_snprintf(tmp, sizeof(tmp), "%lld", end);
            g_signal_handlers_block_by_func(w->entry_end, G_CALLBACK(on_entry_end_changed), w);
            gtk_entry_set_text(GTK_ENTRY(w->entry_end), tmp);
            g_signal_handlers_unblock_by_func(w->entry_end, G_CALLBACK(on_entry_end_changed), w);
        }
    }

    if (w->label_free) {
        long long left = (w->orig_size + w->orig_free) - input_mib;
        if (left < 0) left = 0;
        char lbl[128];
        g_snprintf(lbl, sizeof(lbl), "Free space after resize: %lld MiB", left);
        gtk_label_set_text(GTK_LABEL(w->label_free), lbl);
    }

    g_signal_handlers_unblock_by_func(w->entry_bytes,   G_CALLBACK(on_entry_bytes_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_gib,     G_CALLBACK(on_entry_gib_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_sectors, G_CALLBACK(on_entry_sectors_changed), w);

    is_updating = 0;
}

static void on_entry_gib_changed(GtkEditable *editable, gpointer user_data) {
    CreateFSWidgets *w = (CreateFSWidgets*)user_data;
    if (w->updating) return;
    w->updating = TRUE;

    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (!text || strlen(text) == 0) { w->updating = FALSE; return; }

    double gib = atof(text);
    long long bytes = (long long)(gib * 1024.0 * 1024.0 * 1024.0);
    long long mib = bytes / (1024 * 1024);
    long long sectors = (bytes + w->sector_size - 1) / w->sector_size;

    g_signal_handlers_block_by_func(w->entry_bytes, G_CALLBACK(on_create_entry_bytes_changed), w);
    g_signal_handlers_block_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
    g_signal_handlers_block_by_func(w->entry_sectors, G_CALLBACK(on_entry_sectors_changed), w);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", bytes);
    gtk_entry_set_text(GTK_ENTRY(w->entry_bytes), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", mib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_mib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", sectors);
    gtk_entry_set_text(GTK_ENTRY(w->entry_sectors), tmp);

    g_signal_handlers_unblock_by_func(w->entry_bytes, G_CALLBACK(on_create_entry_bytes_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_sectors, G_CALLBACK(on_entry_sectors_changed), w);

    if (w->label_free) {
        long long mib = (long long)(gib * 1024.0);
        long long left = w->orig_free - mib;
        if (left < 0) left = 0;
        char lbl[128];
        g_snprintf(lbl, sizeof(lbl), "Free space after creation: %lld MiB (%.2f GiB)", 
                   left, (double)left / 1024.0);
        gtk_label_set_text(GTK_LABEL(w->label_free), lbl);
}

    w->updating = FALSE;
}

static void on_entry_sectors_changed(GtkEditable *editable, gpointer user_data) {
    CreateFSWidgets *w = (CreateFSWidgets*)user_data;
    if (w->updating) return;
    w->updating = TRUE;

    const char *text = gtk_entry_get_text(GTK_ENTRY(editable));
    if (!text || strlen(text) == 0) { w->updating = FALSE; return; }

    long long sectors = atoll(text);
    long long bytes = sectors * w->sector_size;
    long long mib = bytes / (1024 * 1024);
    double gib = (double)bytes / (1024.0 * 1024.0 * 1024.0);

    g_signal_handlers_block_by_func(w->entry_bytes, G_CALLBACK(on_create_entry_bytes_changed), w);
    g_signal_handlers_block_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
    g_signal_handlers_block_by_func(w->entry_gib, G_CALLBACK(on_entry_gib_changed), w);

    char tmp[64];
    g_snprintf(tmp, sizeof(tmp), "%lld", bytes);
    gtk_entry_set_text(GTK_ENTRY(w->entry_bytes), tmp);
    g_snprintf(tmp, sizeof(tmp), "%lld", mib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_mib), tmp);
    g_snprintf(tmp, sizeof(tmp), "%.2f", gib);
    gtk_entry_set_text(GTK_ENTRY(w->entry_gib), tmp);

    g_signal_handlers_unblock_by_func(w->entry_bytes, G_CALLBACK(on_create_entry_bytes_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_mib, G_CALLBACK(on_create_entry_mib_changed), w);
    g_signal_handlers_unblock_by_func(w->entry_gib, G_CALLBACK(on_entry_gib_changed), w);

    if (w->label_free) {
        long long mib = bytes / (1024 * 1024);
        long long left = w->orig_free - mib;
        if (left < 0) left = 0;
        char lbl[128];
        g_snprintf(lbl, sizeof(lbl), "Free space after creation: %lld MiB (%.2f GiB)", 
                   left, (double)left / 1024.0);
        gtk_label_set_text(GTK_LABEL(w->label_free), lbl);
}

    w->updating = FALSE;
}

void on_resize_partition_activate(GtkWidget *menuitem, gpointer user_data) {

    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *fstype = NULL, *size = NULL;

    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;

    gtk_tree_model_get(model, &iter,
                       COL_NAME, &partition_name,
                       COL_FSTYPE, &fstype,
                       COL_SIZE, &size,
                       -1);

    gchar *disk_name = get_disk_from_partition(partition_name);
    if (!disk_name) return;

    gchar *device_disk = g_strdup_printf("/dev/%s", disk_name);

    gchar *part_num = NULL;

    if (g_str_has_prefix(partition_name, "nvme") ||
        g_str_has_prefix(partition_name, "mmcblk"))
    {
        gchar *p = g_strrstr(partition_name, "p");
        if (p && isdigit(*(p+1)))
            part_num = g_strdup(p + 1);
    }
    else {
        const gchar *p = partition_name;
        while (*p && !isdigit(*p)) p++;
        if (*p) part_num = g_strdup(p);
    }

    int sector_size = 512;
    {
        gchar *ss_cmd = g_strdup_printf("blockdev --getss %s", device_disk);
        FILE *ss_fp = popen(ss_cmd, "r");
        if (ss_fp) {
            if (fscanf(ss_fp, "%d", &sector_size) != 1)
                sector_size = 512;
            pclose(ss_fp);
        }
        g_free(ss_cmd);
    }

    long long start_mib = 0, end_mib = 0, orig_size_mib = 0;
    long long start_sector = 0, end_sector = 0;

    if (part_num) {
        gchar *cmd = g_strdup_printf(
            "LC_ALL=C parted -m %s unit MiB print "
            "| awk -F: -v N=\"%s\" '$1==N {gsub(/MiB$/, \"\", $2); "
            "gsub(/MiB$/, \"\", $3); print $2 \" \" $3}'",
            device_disk, part_num);

        FILE *fp = popen(cmd, "r");
        if (fp) {
            char buf[256] = {0};
            if (fgets(buf, sizeof(buf), fp)) {
                char *tok = strtok(buf, " \t\n");
                if (tok) start_mib = atoll(tok);

                tok = strtok(NULL, " \t\n");
                if (tok) end_mib = atoll(tok);

                start_sector = (start_mib * 1024LL * 1024LL) / sector_size;
                end_sector   = (end_mib   * 1024LL * 1024LL) / sector_size;

                orig_size_mib = end_mib - start_mib;
            }
            pclose(fp);
        }
        g_free(cmd);
    }

    long long max_free = 0;
    {
        gchar *command = g_strdup_printf(
            "LC_ALL=C parted -m %s unit MiB print free", device_disk);

        FILE *fp = popen(command, "r");
        if (fp) {
            char line[512];
            gboolean in_table = FALSE;
            long long prev_end = 0;
            gboolean found_partition = FALSE;

            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "Number") && strstr(line, "File system")) {
                    in_table = TRUE;
                    continue;
                }
                if (!in_table) continue;

                gchar *trim = g_strstrip(line);
                if (!trim[0]) continue;

                char num[32], st[64], en[64], size_s[64], type[64];
                int p = sscanf(trim, "%31[^:]:%63[^:]:%63[^:]:%63[^:]:%63[^\n]",
                               num, st, en, size_s, type);

                if (p >= 4) {
                    long long block_start = atoll(st);
                    long long block_end   = atoll(en);
                    gchar *block_type = type;

                    if (found_partition && g_str_has_prefix(block_type, "Free Space")) {
                        max_free = block_end - block_start;
                        break;
                    }

                    if (num && part_num && g_strcmp0(num, part_num) == 0) {
                        found_partition = TRUE;
                        prev_end = block_end;
                    }
                }
            }
            pclose(fp);
        }
        if (max_free < 0) max_free = 0;
        g_free(command);
    }

    GtkWidget *main_window = GTK_WIDGET(
        gtk_widget_get_toplevel(GTK_WIDGET(tree_view)));

    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Resize Partition",
        GTK_IS_WINDOW(main_window) ? GTK_WINDOW(main_window) : NULL,
        GTK_DIALOG_MODAL,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Resize", GTK_RESPONSE_ACCEPT,
        NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 540, 420);
    GtkWidget *area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *label1 = gtk_label_new(
        "Resize Partition (Bytes / MiB / GiB / Sectors):");

    GtkWidget *entry_bytes   = gtk_entry_new();
    GtkWidget *entry_mib     = gtk_entry_new();
    GtkWidget *entry_gib     = gtk_entry_new();
    GtkWidget *entry_sectors = gtk_entry_new();

    GtkWidget *label_free = gtk_label_new(NULL);

    GtkWidget *label2 = gtk_label_new("Or specify first and last sector:");

    GtkWidget *entry_start = gtk_entry_new();
    GtkWidget *entry_end   = gtk_entry_new();

    GtkWidget *reminder = gtk_label_new(
        "Note: When shrinking a partition, the console will ask 'Yes/No'. "
        "Type 'Yes' and press Enter to confirm.");
    gtk_widget_set_halign(reminder, GTK_ALIGN_START);

    GtkWidget *alignment_info = gtk_label_new(
        "For best compatibility and performance, the start sector should be 2048 or a multiple of 2048 (1 MiB\n"
        "alignment). This ensures proper partition alignment on modern storage devices.");
    gtk_widget_set_halign(alignment_info, GTK_ALIGN_START);

    {
        char tmp[128];

        g_snprintf(tmp, sizeof(tmp), "%lld",
                   orig_size_mib * 1024LL * 1024LL);
        gtk_entry_set_text(GTK_ENTRY(entry_bytes), tmp);

        g_snprintf(tmp, sizeof(tmp), "%lld", orig_size_mib);
        gtk_entry_set_text(GTK_ENTRY(entry_mib), tmp);

        double gib = (double)orig_size_mib / 1024.0;
        g_snprintf(tmp, sizeof(tmp), "%.2f", gib);
        gtk_entry_set_text(GTK_ENTRY(entry_gib), tmp);

        long long sectors = (end_sector - start_sector);
        g_snprintf(tmp, sizeof(tmp), "%lld", sectors);
        gtk_entry_set_text(GTK_ENTRY(entry_sectors), tmp);

        g_snprintf(tmp, sizeof(tmp), "%lld", start_sector);
        gtk_entry_set_text(GTK_ENTRY(entry_start), tmp);

        g_snprintf(tmp, sizeof(tmp), "%lld", end_sector);
        gtk_entry_set_text(GTK_ENTRY(entry_end), tmp);

        if (max_free > 0) {
            g_snprintf(tmp, sizeof(tmp),
                       "Free space: %lld MiB", max_free);
            gtk_label_set_text(GTK_LABEL(label_free), tmp);
        } else {
            gtk_label_set_text(GTK_LABEL(label_free),
                               "Free space: unknown");
        }
    }

    gtk_box_pack_start(GTK_BOX(area), label1, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(area), entry_bytes,   FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(area), entry_mib,     FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(area), entry_gib,     FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(area), entry_sectors, FALSE, FALSE, 2);

    gtk_box_pack_start(GTK_BOX(area), label_free, FALSE, FALSE, 6);
    gtk_box_pack_start(GTK_BOX(area), label2, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(area), entry_start, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(area), entry_end,   FALSE, FALSE, 2);

    gtk_box_pack_start(GTK_BOX(area), alignment_info, FALSE, FALSE, 8);
    gtk_box_pack_start(GTK_BOX(area), reminder, FALSE, FALSE, 8);

    ResizeWidgets *widgets = g_new0(ResizeWidgets, 1);

    widgets->label_free   = label_free;
    widgets->orig_free    = max_free;
    widgets->orig_size    = orig_size_mib;
    widgets->sector_size  = sector_size;

    widgets->entry_bytes   = entry_bytes;
    widgets->entry_mib     = entry_mib;
    widgets->entry_gib     = entry_gib;
    widgets->entry_sectors = entry_sectors;
    widgets->entry_start   = entry_start;
    widgets->entry_end     = entry_end;

    g_signal_connect(entry_bytes,   "changed",
                     G_CALLBACK(on_entry_bytes_changed),   widgets);
    g_signal_connect(entry_mib,     "changed",
                     G_CALLBACK(on_entry_mib_changed),     widgets);
    g_signal_connect(entry_gib,     "changed",
                     G_CALLBACK(on_entry_gib_changed),     widgets);
    g_signal_connect(entry_sectors, "changed",
                     G_CALLBACK(on_entry_sectors_changed), widgets);
    g_signal_connect(entry_start,   "changed",
                     G_CALLBACK(on_entry_start_changed),   widgets);
    g_signal_connect(entry_end,     "changed",
                     G_CALLBACK(on_entry_end_changed),     widgets);

    on_entry_mib_changed(GTK_EDITABLE(entry_mib), widgets);

    gtk_widget_show_all(dialog);

    g_object_set_data(G_OBJECT(dialog), "widgets", widgets);
    g_object_set_data(G_OBJECT(dialog), "tree_view", tree_view);
    g_object_set_data(G_OBJECT(dialog), "partition_name", partition_name);
    g_object_set_data(G_OBJECT(dialog), "fstype", fstype);
    g_object_set_data(G_OBJECT(dialog), "size", size);

    g_object_set_data(G_OBJECT(dialog), "device_disk", device_disk);
    g_object_set_data(G_OBJECT(dialog), "entry_bytes", entry_bytes);
    g_object_set_data(G_OBJECT(dialog), "entry_mib", entry_mib);
    g_object_set_data(G_OBJECT(dialog), "entry_gib", entry_gib);
    g_object_set_data(G_OBJECT(dialog), "entry_sectors", entry_sectors);
    g_object_set_data(G_OBJECT(dialog), "entry_start", entry_start);
    g_object_set_data(G_OBJECT(dialog), "entry_end", entry_end);

    g_signal_connect(dialog, "response",
                     G_CALLBACK(on_resize_dialog_response), NULL);
    g_signal_connect(dialog, "destroy",
                     G_CALLBACK(on_resize_dialog_destroy), NULL);

    if (part_num) g_free(part_num);
    g_free(disk_name);
}

static void on_resize_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data) {
    ResizeWidgets *widgets = g_object_get_data(G_OBJECT(dialog), "widgets");
    GtkTreeView *tree_view = g_object_get_data(G_OBJECT(dialog), "tree_view");
    gchar *partition_name = g_object_get_data(G_OBJECT(dialog), "partition_name");
    gchar *fstype = g_object_get_data(G_OBJECT(dialog), "fstype");
    GtkWidget *entry_mib = g_object_get_data(G_OBJECT(dialog), "entry_mib");

    if (response_id != GTK_RESPONSE_ACCEPT) {
        gtk_widget_destroy(GTK_WIDGET(dialog));
        return;
    }

    const gchar *size_str = gtk_entry_get_text(GTK_ENTRY(entry_mib));
    if (!size_str || strlen(size_str) == 0) goto cleanup_and_close_dialog;

    gchar *disk_name = get_disk_from_partition(partition_name);
    if (!disk_name) goto cleanup_and_close_dialog;
    gchar *disk_path = g_strdup_printf("/dev/%s", disk_name);

    gchar *part_num = NULL;
    if (g_str_has_prefix(partition_name, "nvme") || g_str_has_prefix(partition_name, "mmcblk")) {
        gchar *p = g_strrstr(partition_name, "p");
        if (p && isdigit(*(p+1))) part_num = g_strdup(p+1);
    } else {
        const gchar *p = partition_name;
        while (*p && !isdigit(*p)) p++;
        if (*p) part_num = g_strdup(p);
    }
    if (!part_num) goto cleanup_and_close_dialog;

    gchar *cmd_sectors = g_strdup_printf(
        "parted -m %s unit s print | grep '^%s:' | cut -d: -f2,3 | tr -d 's'",
        disk_path, part_num
    );
    FILE *fp = popen(cmd_sectors, "r");
    g_free(cmd_sectors);

    long long start_sector = 0, current_end_sector = 0;
    if (fp && fscanf(fp, "%lld:%lld", &start_sector, &current_end_sector) != 2) {
        start_sector = current_end_sector = 0;
    }
    if (fp) pclose(fp);

    gchar *tmp = g_strdup(size_str);
    gchar *t = g_strstrip(tmp);
    while (strlen(t) > 0 && strchr("BbIiMm ", t[strlen(t)-1])) t[strlen(t)-1] = '\0';
    long long target_mib = atoll(t);
    g_free(tmp);
    if (target_mib <= 0) goto cleanup_and_close_dialog;

    long long sector_size = widgets && widgets->sector_size > 0 ? widgets->sector_size : 512;
    long long target_sectors = (target_mib * 1024LL * 1024LL + sector_size - 1) / sector_size;
    long long final_end_sector = start_sector + target_sectors - 1;

    gchar *cmd_disk_size = g_strdup_printf("blockdev --getsz %s", disk_path);
    FILE *fd = popen(cmd_disk_size, "r");
    g_free(cmd_disk_size);
    long long disk_sectors = 0;
    if (fd && fscanf(fd, "%lld", &disk_sectors) != 1) disk_sectors = 0;
    if (fd) pclose(fd);

    if (disk_sectors > 0 && final_end_sector >= disk_sectors)
        final_end_sector = disk_sectors - 1;

    gchar *part_path = NULL;
    if (g_str_has_prefix(disk_name, "nvme") || g_str_has_prefix(disk_name, "mmcblk"))
        part_path = g_strdup_printf("/dev/%sp%s", disk_name, part_num);
    else
        part_path = g_strdup_printf("/dev/%s%s", disk_name, part_num);

    gboolean shrinking = final_end_sector < current_end_sector;

    GString *script = g_string_new(NULL);
    g_string_append_printf(script,
        "#!/bin/bash\nset -e\n\n"
        "before_start=%lld\n"
        "before_end=%lld\n"
        "before_size=$((before_end - before_start + 1))\n"
        "before_mib=$((before_size*%lld/1024/1024))\n"
        "before_gib=$((before_mib/1024))\n\n"
        "echo \"======================================\"\n"
        "echo \" BEFORE\"\n"
        "echo \" Start sector : $before_start\"\n"
        "echo \" End sector   : $before_end\"\n"
        "echo \" Size (sect)  : $before_size  ($before_mib MiB / ~$before_gib GiB)\"\n"
        "echo \"======================================\"\n\n",
        start_sector, current_end_sector, sector_size
    );

    g_string_append_printf(script, "umount %s 2>/dev/null || true\n\n", part_path);

    if (shrinking && g_str_has_prefix(fstype, "ext")) {
        g_string_append_printf(script,
            "echo 'Running e2fsck...'\n"
            "e2fsck -fy %s\n"
            "echo 'Shrinking filesystem...'\n"
            "resize2fs %s %lldM\n\n",
            part_path, part_path, target_mib
        );

        g_string_append_printf(script,
            "blk_cnt=$(dumpe2fs -h %s | awk '/Block count:/ {print $3}')\n"
            "blk_sz=$(dumpe2fs -h %s | awk '/Block size:/ {print $3}')\n"
            "fs_sectors=$(( (blk_cnt * blk_sz + %lld - 1)/%lld ))\n"
            "new_end=$(( %lld + fs_sectors - 1 ))\n"
            "echo \"Filesystem end sector: $new_end\"\n\n",
            part_path, part_path, sector_size, sector_size, start_sector
        );

        g_string_append_printf(script,
            "echo 'Resizing partition (shrink)...'\n"
            "parted %s resizepart %s ${new_end}s\n"
            "echo 'Partition shrink completed.'\n\n",
            disk_path, part_num
        );
    }

    if (!shrinking) {
        g_string_append_printf(script,
            "echo 'Extending partition...'\n"
            "parted --script %s resizepart %s %llds\n"
            "echo 'Partition extend completed.'\n\n",
            disk_path, part_num, final_end_sector
        );

        if (g_str_has_prefix(fstype, "ext")) {
            g_string_append_printf(script,
                "echo 'Updating filesystem...'\n"
                "e2fsck -fy %s || true\n"
                "resize2fs %s || true\n\n",
                part_path, part_path
            );
        }
    }

    g_string_append_printf(script,
        "udevadm settle && partprobe %s || blockdev --rereadpt %s\\n"
        "after_start=$(parted -m %s unit s print | grep '^%s:' | cut -d: -f2 | tr -d 's')\n"
        "after_end=$(parted -m %s unit s print | grep '^%s:' | cut -d: -f3 | tr -d 's')\n"
        "after_size=$((after_end - after_start + 1))\n"
        "after_mib=$((after_size*%lld/1024/1024))\n"
        "after_gib=$((after_mib/1024))\n\n"
        "echo \"======================================\"\n"
        "echo \" AFTER\"\n"
        "echo \" Start sector : $after_start\"\n"
        "echo \" End sector   : $after_end\"\n"
        "echo \" Size (sect)  : $after_size  ($after_mib MiB / ~$after_gib GiB)\"\n"
        "echo \"======================================\"\n"
        "echo \"Operation completed.\"\n"
        "rm -f \"$0\"\n",
        disk_path, disk_path, part_num, disk_path, part_num, sector_size
    );

    gchar *tmp_path = g_strdup_printf("/tmp/driveassistify_resize_%ld.sh", (long)getpid());
    g_file_set_contents(tmp_path, script->str, -1, NULL);
    chmod(tmp_path, 0755);

    run_command_in_terminal(tree_view, tmp_path);

    g_string_free(script, TRUE);
    g_free(tmp_path);
    g_free(part_path);
    g_free(part_num);
    g_free(disk_name);
    g_free(disk_path);

cleanup_and_close_dialog:
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

static void on_grub_terminal_exited(VteTerminal *terminal, gint status, gpointer user_data) {
    GtkWidget *window = GTK_WIDGET(user_data);
    gtk_window_set_title(GTK_WINDOW(window), "Command Finished - You can close this window");
}

void on_grub_uefi_install_activate(GtkWidget *menuitem, gpointer user_data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *partition_name = NULL, *mountpoint = NULL;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {

        gtk_tree_model_get(model, &iter,
                           COL_NAME, &partition_name,
                           COL_MOUNTPOINT, &mountpoint,
                           -1);

        gchar *device_path        = g_strdup_printf("/dev/%s", partition_name);
        gchar *mount_dir          = g_strdup("/mnt/driveassistify_grub");
        gchar *quoted_device_path = g_shell_quote(device_path);
        gchar *quoted_mountpoint  = mountpoint ? g_shell_quote(mountpoint) : g_strdup("''");

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This is a potentially destructive operation!\n\n"
            "Installing GRUB2 to a UEFI partition.\n"
            "If you select the wrong partition, your system may become unbootable.\n\n"
            "Target partition: %s\n\n"
            "Are you sure you want to continue?",
            device_path
        );

        gint response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (response == GTK_RESPONSE_OK) {

            gchar *command = g_strdup_printf(
                "echo '========================================='; "
                "echo 'Starting UEFI GRUB installation'; "
                "echo 'Target device: %s'; "
                "echo 'Mount directory: %s'; "
                "echo '-----------------------------------------'; "

                "echo 'Unmounting previous mount (if any)...'; "

                "if mount | grep -q '%s '; then "
                "  echo 'Trying normal umount with timeout...'; "
                "  for i in 1 2; do "
                "    timeout 15 umount %s && break || { "
                "      echo 'Attempt $i failed, retrying in 2s...'; "
                "      sleep 2; "
                "    }; "
                "  done; "
                "  if mount | grep -q '%s '; then "
                "    echo 'ERROR: Could not unmount the device.'; "
                "    echo 'You may need to unmount manually using:'; "
                "    echo '   umount %s'; "
                "    echo '   umount -l %s   (lazy)'; "
                "    echo '   umount -f %s   (force)'; "
                "    echo 'Aborting.'; "
                "    echo; "
                "    echo 'This window can now be closed.'; "
                "    read -p 'Press Enter to close...'; "
                "    exit 1; "
                "  fi; "
                "else "
                "  echo 'Device not mounted — OK'; "
                "fi; "

                "echo 'Creating temporary mount directory...'; "
                "mkdir -p %s; "

                "echo 'Mounting target partition (vfat)...'; "
                "mount -t vfat %s %s || { "
                "  echo 'ERROR: Failed to mount partition! Aborting.'; "
                "  echo; "
                "  echo 'This window can now be closed.'; "
                "  read -p 'Press Enter to close...'; "
                "  exit 1; "
                "}; "

                "echo 'Ensuring boot directory exists...'; "
                "mkdir -p %s/boot; "

                "echo 'Running grub-install...'; "
                "grub-install --target=x86_64-efi "
                "             --efi-directory=%s "
                "             --boot-directory=%s/boot "
                "             --removable --no-nvram || { "
                "    echo 'ERROR: grub-install failed! Unmounting...'; "
                "    timeout 15 umount %s 2>/dev/null; "
                "    echo; "
                "    echo 'This window can now be closed.'; "
                "    read -p 'Press Enter to close...'; "
                "    exit 1; "
                "}; "

                "echo 'Unmounting...'; "
                "timeout 15 umount %s; "

                "echo 'Removing temporary directory...'; "
                "rmdir %s; "

                "echo '-----------------------------------------'; "
                "echo 'UEFI GRUB installation finished successfully.'; "
                "echo '========================================='; "
                "echo; "
                "read -p 'Press Enter to close...';",

                device_path,
                mount_dir,

                device_path,
                device_path,
                device_path,
                device_path,
                device_path,
                device_path,

                mount_dir,
                quoted_device_path,
                mount_dir,
                mount_dir,
                mount_dir,
                mount_dir,
                mount_dir,
                mount_dir,
                mount_dir
            );

            pid_t pid = fork();
            
            if (pid == 0) {
                const gchar *terminals[] = {"gnome-terminal", "konsole", "xfce4-terminal", 
                                           "mate-terminal", "lxterminal", "xterm", NULL};
                
                for (int i = 0; terminals[i]; i++) {
                    gchar *term_bin = g_find_program_in_path(terminals[i]);
                    if (term_bin) {
                        if (g_strcmp0(terminals[i], "gnome-terminal") == 0) {
                            execlp(term_bin, term_bin, "--", "bash", "-c", command, NULL);
                        } else {
                            execlp(term_bin, term_bin, "-e", "bash", "-c", command, NULL);
                        }
                        g_free(term_bin);
                    }
                }
                
                exit(1);
            } else if (pid > 0) {
                g_free(command);
            } else {
                g_warning("Failed to fork process");
                GtkWidget *err = gtk_message_dialog_new(
                    GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(tree_view))),
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Failed to create new process."
                );
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                g_free(command);
            }
        }

        g_free(quoted_device_path);
        g_free(quoted_mountpoint);
        g_free(mount_dir);
        g_free(device_path);
        g_free(partition_name);
        g_free(mountpoint);
    }
}

void on_grub_mbr_install_activate(GtkWidget *menuitem, gpointer user_data) {
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
        gchar *quoted_mountpoint = mountpoint ? g_shell_quote(mountpoint) : g_strdup("''");

        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This is a potentially destructive operation!\n\n"
            "Installing GRUB2 to the MBR rewrites boot code on the disk.\n"
            "If you select the wrong partition or disk, the system may become unbootable.\n\n"
            "Target partition: %s\n\n"
            "Are you sure you want to continue?",
            device_path
        );
        gint response = gtk_dialog_run(GTK_DIALOG(warn));
        gtk_widget_destroy(warn);

        if (response == GTK_RESPONSE_OK) {

            gchar *command = g_strdup_printf(
                "echo '========================================='; "
                "echo 'Starting GRUB2 MBR installation'; "
                "echo 'Target partition: %s'; "
                "echo 'Target disk: %s'; "
                "echo 'Mount directory: %s'; "
                "echo '-----------------------------------------'; "

                "echo 'Unmounting previous mount (if any)...'; "

                "if mount | grep -q '%s '; then "
                "  echo 'Trying normal umount with timeout...'; "
                "  for i in 1 2; do "
                "    timeout 15 umount %s && break || { "
                "      echo 'Attempt $i failed, retrying in 2s...'; "
                "      sleep 2; "
                "    }; "
                "  done; "
                "  if mount | grep -q '%s '; then "
                "    echo 'ERROR: Could not unmount the device.'; "
                "    echo 'You may need to unmount manually using:'; "
                "    echo '   umount %s'; "
                "    echo '   umount -l %s   (lazy)'; "
                "    echo '   umount -f %s   (force)'; "
                "    echo 'Aborting.'; "
                "    echo; "
                "    echo 'This window can now be closed.'; "
                "    read -p 'Press Enter to close...'; "
                "    exit 1; "
                "  fi; "
                "else "
                "  echo 'Device not mounted — OK'; "
                "fi; "

                "echo 'Creating temporary mount directory...'; "
                "mkdir -p %s; "

                "echo 'Mounting target partition...'; "
                "mount %s %s || { "
                "  echo 'ERROR: Failed to mount partition! Aborting.'; "
                "  echo; "
                "  echo 'This window can now be closed.'; "
                "  read -p 'Press Enter to close...'; "
                "  exit 1; "
                "}; "

                "echo 'Running grub-install (i386-pc)...'; "
                "grub-install --target=i386-pc --boot-directory=%s/boot %s || { "
                "    echo 'ERROR: grub-install failed! Unmounting...'; "
                "    timeout 15 umount %s; "
                "    echo; "
                "    echo 'This window can now be closed.'; "
                "    read -p 'Press Enter to close...'; "
                "    exit 1; "
                "}; "

                "echo 'Unmounting...'; "
                "timeout 15 umount %s; "

                "echo 'Removing temporary directory...'; "
                "rmdir %s; "

                "echo '-----------------------------------------'; "
                "echo 'GRUB2 MBR installation finished successfully.'; "
                "echo '========================================='; "
                "echo; "
                "read -p 'Press Enter to close...';",
                
                device_path,
                disk_path,
                mount_dir,

                device_path,
                device_path,
                device_path,
                device_path,
                device_path,
                device_path,

                quoted_mount_dir,
                quoted_device_path,
                quoted_mount_dir,
                mount_dir,
                quoted_disk_path,
                quoted_mount_dir,
                quoted_mount_dir,
                quoted_mount_dir
            );

            pid_t pid = fork();
            
            if (pid == 0) {
                const gchar *terminals[] = {"gnome-terminal", "konsole", "xfce4-terminal", 
                                           "mate-terminal", "lxterminal", "xterm", NULL};
                
                for (int i = 0; terminals[i]; i++) {
                    gchar *term_bin = g_find_program_in_path(terminals[i]);
                    if (term_bin) {
                        if (g_strcmp0(terminals[i], "gnome-terminal") == 0) {
                            execlp(term_bin, term_bin, "--", "bash", "-c", command, NULL);
                        } else {
                            execlp(term_bin, term_bin, "-e", "bash", "-c", command, NULL);
                        }
                        g_free(term_bin);
                    }
                }
                
                exit(1);
            } else if (pid > 0) {
                g_free(command);
            } else {
                g_warning("Failed to fork process");
                GtkWidget *err = gtk_message_dialog_new(
                    GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(tree_view))),
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_OK,
                    "Failed to create new process."
                );
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
                g_free(command);
            }
        }

        g_free(quoted_device_path);
        g_free(quoted_disk_path);
        g_free(quoted_mountpoint);
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

            run_command_in_terminal(tree_view, parted_cmd);
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
                        "sudo dd if=%s of=%s bs=4M status=progress",
                        quoted_device, quoted_device, quoted_file
                    );
                } else {
                    command = g_strdup_printf(
                        "sudo dd if=%s of=%s bs=4M status=progress",
                        quoted_device, quoted_file
                    );
                }

                run_command_in_terminal(tree_view, command);

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
                
                gchar *disk_name = get_disk_from_partition(partition_name);
                gchar *disk_path = g_strdup_printf("/dev/%s", disk_name);
                gchar *quoted_disk = g_shell_quote(disk_path);

                gchar *command = NULL;
                if (mountpoint && strlen(mountpoint) > 0 && strcmp(mountpoint, "N/A") != 0 && strcmp(mountpoint, "-") != 0) {
                    command = g_strdup_printf(
                        "echo 'WARNING! Do NOT mount this partition during restore, otherwise the data may be corrupted.'; "
                        "umount %s 2>/dev/null; "
                        "sudo dd if=%s of=%s bs=4M status=progress; "
                        "echo 'Updating partition table...'; sudo partprobe %s || sudo blockdev --rereadpt %s; "
                        "sleep 1; "
                        "echo 'Partition restored successfully!'",
                        quoted_device, quoted_file, quoted_device,
                        quoted_disk, quoted_disk
                    );
                } else {
                    command = g_strdup_printf(
                        "echo 'WARNING! Do NOT mount this partition during restore, otherwise the data may be corrupted.'; "
                        "sudo dd if=%s of=%s bs=4M status=progress; "
                        "echo 'Updating partition table...'; sudo partprobe %s || sudo blockdev --rereadpt %s; "
                        "sleep 1; "
                        "echo 'Partition restored successfully!'",
                        quoted_file, quoted_device,
                        quoted_disk, quoted_disk
                    );
                }

                run_command_in_terminal(tree_view, command);

                g_free(command);
                g_free(quoted_device);
                g_free(quoted_file);
                g_free(quoted_disk);
                g_free(disk_path);
                g_free(disk_name);
            }

            g_free(device_path);
            g_free(filename);
        }
        gtk_widget_destroy(dialog);
        g_free(partition_name);
        g_free(mountpoint);
    }
}

void on_delete_partition_table_activate(GtkWidget *menuitem, gpointer user_data) {
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

        gchar *device_path = g_strdup_printf("/dev/%s", disk_name);
        GtkWidget *dialog = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "Delete partition table from %s?\n\n"
            "This will REMOVE ALL PARTITIONS and data from disk!\n\n"
            "Are you sure you want to continue?",
            device_path
        );
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (result == GTK_RESPONSE_OK) {
            gchar *quoted_device = g_shell_quote(device_path);
            gchar *basename_dev = g_path_get_basename(device_path);

            GtkWidget *warn = gtk_message_dialog_new(
                NULL,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK_CANCEL,
                "FINAL WARNING!\n\n"
                "Delete partition table from %s?\n\n"
                "ALL PARTITIONS AND DATA WILL BE LOST!\n\n"
                "This cannot be undone!",
                device_path
            );
            gint resp = gtk_dialog_run(GTK_DIALOG(warn));
            gtk_widget_destroy(warn);

            if (resp == GTK_RESPONSE_OK) {
                gchar *command = g_strdup_printf(
                    "echo '1. Unmounting all partitions...'; "
                    "for part in $(lsblk -ln -o NAME %s | grep -v ^%s$); do "
                    "umount /dev/$part 2>/dev/null || true; "
                    "sudo wipefs -a /dev/$part 2>/dev/null || true; "
                    "done; "
                    "echo '2. Wiping signatures from disk...'; "
                    "sudo wipefs -a %s 2>/dev/null || true; "
                    "echo '3. Creating empty partition table...'; "
                    "sudo dd if=/dev/zero of=%s bs=1M count=10 conv=notrunc 2>/dev/null && "
                    "sudo udevadm settle && sudo partprobe %s || sudo blockdev --rereadpt %s && sleep 5 && "
                    "echo '4. Verifying...'; "
                    "[ -z \"$(sudo parted -s %s print 2>&1 | grep 'Number')\" ] && "
                    "echo 'Partition table deleted successfully from %s!' || "
                    "echo 'Failed: partition table not fully deleted from %s'",
                    quoted_device, basename_dev,
                    quoted_device,
                    device_path,
                    device_path, device_path,
                    device_path,
                    device_path, device_path
                );
                run_command_in_terminal(tree_view, command);
                g_free(command);
            }
            g_free(basename_dev);
            g_free(quoted_device);
        }
        g_free(device_path);
        g_free(disk_name);
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
        gchar *disk_name = get_disk_from_partition(partition_name);
        gchar *disk_path = g_strdup_printf("/dev/%s", disk_name);
        
        gchar *part_num = NULL;
        if (g_str_has_prefix(partition_name, "nvme") || g_str_has_prefix(partition_name, "mmcblk")) {
            gchar *p = g_strrstr(partition_name, "p");
            if (p && isdigit(*(p+1))) {
                part_num = g_strdup(p + 1);
            }
        } else {
            const gchar *p = partition_name;
            while (*p && !isdigit(*p)) p++;
            if (*p) part_num = g_strdup(p);
        }
        if (!part_num) {
            GtkWidget *err = gtk_message_dialog_new(
                NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                "Cannot determine partition number.");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            g_free(device_path);
            g_free(disk_name);
            g_free(disk_path);
            g_free(partition_name);
            if (mountpoint) g_free(mountpoint);
            return;
        }
        GtkWidget *warn = gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK_CANCEL,
            "WARNING: This will irreversibly delete partition %s from %s.\n\n"
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
                    "echo 'Unmounting %s...'; timeout 5 sudo umount %s 2>/dev/null || echo 'Unmount warning ignored'; "
                    "echo 'Deleting partition %s (number %s)...'; "
                    "if sudo parted -s %s print 2>/dev/null | grep -q 'Partition Table: gpt'; then "
                    "  sudo parted -s %s rm %s; "
                    "else "
                    "  echo -e 'd\\n%s\\nw\\n' | sudo fdisk %s 2>&1 | grep -E 'altered|ioctl|Syncing'; "
                    "fi && "
                    "sudo udevadm settle && sudo partprobe %s || sudo blockdev --rereadpt %s && "
                    "sleep 2 && "
                    "echo 'Partition deleted and partition table re-read.'",
                    device_path, device_path,
                    partition_name, part_num,
                    disk_path,
                    disk_path, part_num,
                    part_num, disk_path,
                    disk_path, disk_path
                );
            } else {
                command = g_strdup_printf(
                    "echo 'Deleting partition %s (number %s)...'; "
                    "if sudo parted -s %s print 2>/dev/null | grep -q 'Partition Table: gpt'; then "
                    "  sudo parted -s %s rm %s; "
                    "else "
                    "  echo -e 'd\\n%s\\nw\\n' | sudo fdisk %s 2>&1 | grep -E 'altered|ioctl|Syncing'; "
                    "fi && "
                    "sudo udevadm settle && sudo partprobe %s || sudo blockdev --rereadpt %s && "
                    "sleep 2 && "
                    "echo 'Partition deleted and partition table re-read.'",
                    partition_name, part_num,
                    disk_path,
                    disk_path, part_num,
                    part_num, disk_path,
                    disk_path, disk_path
                );
            }
            run_command_in_terminal(tree_view, command);
            g_free(command);
        }
        g_free(part_num);
        g_free(device_path);
        g_free(disk_path);
        g_free(disk_name);
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
                    "echo 'Unmounting %s...'; timeout 5 sudo umount %s 2>/dev/null || echo 'Unmount warning ignored'; "
                    "sudo udevadm settle; "
                    "echo 'Shredding %s...'; "
                    "sudo shred -v -n 3 -z %s 2>/dev/null && "
                    "sudo udevadm settle && "
                    "echo 'Filesystem destroyed.'",
                    device_path, device_path,
                    device_path, device_path
                );
            } else {
                command = g_strdup_printf(
                    "sudo udevadm settle; "
                    "echo 'Shredding %s...'; "
                    "sudo shred -v -n 3 -z %s 2>/dev/null && "
                    "sudo udevadm settle && "
                    "echo 'Filesystem destroyed.'",
                    device_path, device_path
                );
            }

            run_command_in_terminal(tree_view, command);
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
                "dd if=/dev/urandom of=%s bs=1M status=progress && sudo udevadm settle && echo 'Disk erased successfully'",
                quoted_device
            );
            run_command_in_terminal(tree_view, command);
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
                "sudo dd if=/dev/urandom of=%s bs=1M status=progress && "
                "sudo dd if=/dev/zero of=%s bs=1M status=progress && "
                "sudo dd if=/dev/zero of=%s bs=1M status=progress && "
                "sudo dd if=/dev/full of=%s bs=1M status=progress && "
                "sudo udevadm settle && "
                "echo 'Disk erased successfully'",
                quoted_device, quoted_device, quoted_device, quoted_device
            );
            run_command_in_terminal(tree_view, command);
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
            char check_cmd[256];
            snprintf(check_cmd, sizeof(check_cmd), 
                "lsblk -ndo TYPE /dev/%s 2>/dev/null | head -1", name);
            FILE *check_fp = popen(check_cmd, "r");
            if (check_fp) {
                char real_type[32] = {0};
                if (fgets(real_type, sizeof(real_type), check_fp)) {
                    char *newline = strchr(real_type, '\n');
                    if (newline) *newline = '\0';
                    if (strcmp(real_type, "part") == 0) {
                        strcpy(type, "part");
                    }
                }
                pclose(check_fp);
            }
            
            if (strcmp(type, "disk") == 0) {
                char check_partitions[256];
                snprintf(check_partitions, sizeof(check_partitions), 
                    "lsblk -nlo TYPE /dev/%s 2>/dev/null | grep -c 'part'", name);
                FILE *part_fp = popen(check_partitions, "r");
                if (part_fp) {
                    char part_count[16] = {0};
                    if (fgets(part_count, sizeof(part_count), part_fp)) {
                        int count = atoi(part_count);
                        if (count > 0 && strlen(fstype) > 0) {
                            fstype[0] = '\0';
                            uuid[0] = '\0';
                        }
                    }
                    pclose(part_fp);
                }
                
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

    GtkWidget *info_item = gtk_menu_item_new_with_label("Device Information (lsblk, fdisk, parted, blkid, dumpe2fs, dosfsck, ntfsinfo, dumpexfat)");
    g_signal_connect(info_item, "activate", G_CALLBACK(on_device_info_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), info_item);

    GtkWidget *show_areas_item = gtk_menu_item_new_with_label("Show Filesystems and Free Space (parted, lsblk)");
    g_signal_connect(show_areas_item, "activate", G_CALLBACK(on_show_disk_areas_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), show_areas_item);

    GtkWidget *smartctl_item = gtk_menu_item_new_with_label("S.M.A.R.T. Information (smartctl)");
    g_signal_connect(smartctl_item, "activate", G_CALLBACK(on_smartctl_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), smartctl_item);

    GtkWidget *read_benchmark_item = gtk_menu_item_new_with_label("Sequential Read Speed Test (dd)");
    g_signal_connect(read_benchmark_item, "activate", G_CALLBACK(on_disk_read_benchmark_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), read_benchmark_item);

    GtkWidget *file_write_item = gtk_menu_item_new_with_label("File Write Speed Test (dd)");
    g_signal_connect(file_write_item, "activate", G_CALLBACK(on_disk_file_write_benchmark_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), file_write_item);

    GtkWidget *raw_write_item = gtk_menu_item_new_with_label("Raw Device Speed Test (dd) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(raw_write_item, "activate", G_CALLBACK(on_disk_raw_write_benchmark_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(info_menu), raw_write_item);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), info_root);

    GtkWidget *scan_menu = gtk_menu_new();
    GtkWidget *scan_root = gtk_menu_item_new_with_label("Scan & Repair");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(scan_root), scan_menu);

    GtkWidget *auto_fsck_item = gtk_menu_item_new_with_label("Check and Repair Filesystem (auto-detect) (e2fsck, dosfsck, ntfsfix) => POTENTIALLY DANGEROUS! May cause data loss if errors are present.");
    g_signal_connect(auto_fsck_item, "activate", G_CALLBACK(on_auto_fsck_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), auto_fsck_item);

    GtkWidget *e2fsck_item = gtk_menu_item_new_with_label("Check and Repair EXT Filesystem (e2fsck) => POTENTIALLY DANGEROUS! May cause data loss if errors are present.");
    g_signal_connect(e2fsck_item, "activate", G_CALLBACK(on_e2fsck_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), e2fsck_item);

    GtkWidget *ext_repair_deep_item = gtk_menu_item_new_with_label("Deep EXT Recovery (e2fsck) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(ext_repair_deep_item, "activate", G_CALLBACK(on_ext_repair_deep_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), ext_repair_deep_item);

    GtkWidget *fat32fix_item = gtk_menu_item_new_with_label("Check and Repair FAT32 Filesystem (dosfsck) => POTENTIALLY DANGEROUS! May cause data loss if errors are present.");
    g_signal_connect(fat32fix_item, "activate", G_CALLBACK(on_fat32fix_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(scan_menu), fat32fix_item);

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

    GtkWidget *create_partition_item = gtk_menu_item_new_with_label("Create Partition (parted, lsblk)");
    g_signal_connect(create_partition_item, "activate", G_CALLBACK(on_show_disk_areas_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), create_partition_item);

    GtkWidget *mkfs_item = gtk_menu_item_new_with_label("Format Filesystem (mkfs) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(mkfs_item, "activate", G_CALLBACK(on_mkfs_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), mkfs_item);

    GtkWidget *resize_item = gtk_menu_item_new_with_label("Resize/Move Partition (ext2/3/4, ntfsresize, fatresize, exfatprogs) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(resize_item, "activate", G_CALLBACK(on_resize_partition_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), resize_item);

    GtkWidget *grub_uefi_install_item = gtk_menu_item_new_with_label("Install GRUB2 Bootloader for UEFI (grub-install) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(grub_uefi_install_item, "activate", G_CALLBACK(on_grub_uefi_install_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), grub_uefi_install_item);

    GtkWidget *grub_mbr_install_item = gtk_menu_item_new_with_label("Install GRUB2 Bootloader for BIOS/MBR (grub-install) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(grub_mbr_install_item, "activate", G_CALLBACK(on_grub_mbr_install_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(fs_menu), grub_mbr_install_item);

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

    GtkWidget *delete_table_item = gtk_menu_item_new_with_label("Delete Partition Table (dd/parted) => DANGEROUS! Think carefully before proceeding!");
    g_signal_connect(delete_table_item, "activate", G_CALLBACK(on_delete_partition_table_activate), tree_view);
    gtk_menu_shell_append(GTK_MENU_SHELL(delete_menu), delete_table_item);

    GtkWidget *delete_item = gtk_menu_item_new_with_label("Delete Partition (parted/fdisk) => DANGEROUS! Think carefully before proceeding!");
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
"Created with the assistance of ChatGPT, Perplexity, and Claude.\n"
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
"Created with the assistance of ChatGPT, Perplexity, and Claude.\n\n"
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
    gtk_window_set_title(GTK_WINDOW(window), "DriveAssistify v1.8");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    set_window_icon(window);

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

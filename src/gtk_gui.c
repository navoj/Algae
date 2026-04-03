/*
 * gtk_gui.c -- GTK+ graphical interface for Algae.
 *
 * Provides a simple GTK3 GUI with:
 *   - A text console (output + input) using GtkTextView
 *   - Menu bar with File (Open, Source, Quit) and Help (About, Help)
 *   - Command entry field with history
 *   - Syntax highlighting for output
 *
 * Compile with:
 *   gcc $(pkg-config --cflags gtk+-3.0) -o algae-gtk gtk_gui.c \
 *       $(pkg-config --libs gtk+-3.0) -I../src
 *
 * Or use the Makefile target: make algae-gtk
 *
 * Copyright (C) 2026  Algae contributors.
 *
 * See the file COPYING for license, warranty, and permission details.
 */

#include <gtk/gtk.h>
#include <glib-unix.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pty.h>
#include <termios.h>
#include <sys/wait.h>
#include <signal.h>

/* PTY master file descriptor for communicating with algae process */
static int pty_master_fd = -1;
static GPid algae_pid = 0;
static guint algae_read_source = 0;

/* GUI widgets */
static GtkWidget *window;
static GtkWidget *text_view;
static GtkTextBuffer *text_buffer;
static GtkWidget *entry;
static GtkWidget *statusbar;

/* Command history */
#define MAX_HISTORY 500
static char *cmd_history[MAX_HISTORY];
static int history_count = 0;
static int history_pos = -1;

static void append_text (const char *text, const char *tag_name);
static void send_command (const char *cmd);
static gboolean read_algae_output (gint fd, GIOCondition cond, gpointer data);
static void start_algae_process (void);
static void stop_algae_process (void);

/* ---- Text buffer management ---- */

static void
append_text (const char *text, const char *tag_name)
{
  GtkTextIter end;
  gtk_text_buffer_get_end_iter (text_buffer, &end);

  if (tag_name)
    gtk_text_buffer_insert_with_tags_by_name (text_buffer, &end,
                                               text, -1, tag_name, NULL);
  else
    gtk_text_buffer_insert (text_buffer, &end, text, -1);

  /* Auto-scroll to bottom */
  GtkTextMark *mark = gtk_text_buffer_get_insert (text_buffer);
  gtk_text_buffer_get_end_iter (text_buffer, &end);
  gtk_text_buffer_move_mark (text_buffer, mark, &end);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (text_view), mark);
}

/* ---- Communication with Algae ---- */

static void
send_command (const char *cmd)
{
  if (algae_pid > 0 && pty_master_fd >= 0)
    {
      ssize_t n;
      n = write (pty_master_fd, cmd, strlen (cmd));
      if (n < 0)
        g_printerr ("algae-gtk: write(cmd) failed: %s\n", strerror (errno));
      n = write (pty_master_fd, "\n", 1);
      if (n < 0)
        g_printerr ("algae-gtk: write(newline) failed: %s\n", strerror (errno));
    }
}

static gboolean
read_algae_output (gint fd, GIOCondition cond, gpointer data)
{
  if (cond & G_IO_IN)
    {
      char buf[4096];
      ssize_t n;

      /* Read all available data in a loop */
      while ((n = read (fd, buf, sizeof (buf) - 1)) > 0)
        {
          buf[n] = '\0';
          append_text (buf, "output");
        }
      if (n == 0)
        {
          /* EOF */
          append_text ("\n[Algae process ended]\n", "error");
          algae_pid = 0;
          pty_master_fd = -1;
          algae_read_source = 0;
          gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, "Algae stopped");
          return G_SOURCE_REMOVE;
        }
      /* n < 0 && errno == EAGAIN → no more data right now, keep watching */
    }

  if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
    {
      /* Drain any remaining data before closing */
      char buf[4096];
      ssize_t n;
      while ((n = read (fd, buf, sizeof (buf) - 1)) > 0)
        {
          buf[n] = '\0';
          append_text (buf, "output");
        }
      append_text ("\n[Connection to Algae lost]\n", "error");
      algae_pid = 0;
      pty_master_fd = -1;
      algae_read_source = 0;
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, "Algae stopped");
      return G_SOURCE_REMOVE;
    }

  return G_SOURCE_CONTINUE;
}

static void
child_exited_cb (GPid pid, gint status, gpointer data)
{
  g_printerr ("algae-gtk: child process exited (status %d)\n", status);
  g_spawn_close_pid (pid);
  if (algae_pid == pid)
    {
      algae_pid = 0;
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, "Algae stopped");
    }
}

static void
start_algae_process (void)
{
  struct termios termp;
  struct winsize winp;

  /* Configure the pseudo-terminal: raw mode, no echo, no signals */
  memset (&termp, 0, sizeof (termp));
  cfmakeraw (&termp);

  /* Keep output post-processing so \n works naturally */
  termp.c_oflag |= OPOST | ONLCR;

  /* No echo — we show commands ourselves in the GUI */
  termp.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

  /* Set a reasonable window size */
  memset (&winp, 0, sizeof (winp));
  winp.ws_row = 40;
  winp.ws_col = 120;

  algae_pid = forkpty (&pty_master_fd, NULL, &termp, &winp);
  if (algae_pid < 0)
    {
      g_printerr ("algae-gtk: forkpty failed: %s\n", strerror (errno));
      append_text ("Failed to start Algae process.\n", "error");
      return;
    }

  if (algae_pid == 0)
    {
      /* Child: exec algae.  The PTY is already stdin/stdout/stderr. */
      execlp ("algae", "algae", "-r", "-n", "-i", (char *) NULL);
      execl ("./algae", "algae", "-r", "-n", "-i", (char *) NULL);
      _exit (127);
    }

  /* Parent: set master fd non-blocking */
  {
    int flags = fcntl (pty_master_fd, F_GETFL, 0);
    if (flags >= 0)
      fcntl (pty_master_fd, F_SETFL, flags | O_NONBLOCK);
  }

  /* Watch for output on the PTY master */
  algae_read_source = g_unix_fd_add (pty_master_fd,
                                      G_IO_IN | G_IO_ERR | G_IO_HUP,
                                      read_algae_output, NULL);

  /* Watch for child exit */
  g_child_watch_add (algae_pid, child_exited_cb, NULL);

  g_printerr ("algae-gtk: Algae process started (pid %d, pty fd %d)\n",
               (int) algae_pid, pty_master_fd);
  append_text ("Algae process started.\n", "info");
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, "Connected to Algae");
}

static void
stop_algae_process (void)
{
  if (algae_pid > 0)
    {
      send_command ("exit()");
      usleep (100000);  /* Give it 100ms to exit cleanly */
      kill (algae_pid, SIGTERM);
      algae_pid = 0;
    }
  if (algae_read_source > 0)
    {
      g_source_remove (algae_read_source);
      algae_read_source = 0;
    }
  if (pty_master_fd >= 0)
    {
      close (pty_master_fd);
      pty_master_fd = -1;
    }
}

/* ---- History management ---- */

static void
add_to_history (const char *cmd)
{
  if (history_count < MAX_HISTORY)
    cmd_history[history_count++] = g_strdup (cmd);
  else
    {
      g_free (cmd_history[0]);
      memmove (cmd_history, cmd_history + 1,
               (MAX_HISTORY - 1) * sizeof (char *));
      cmd_history[MAX_HISTORY - 1] = g_strdup (cmd);
    }
  history_pos = history_count;
}

/* ---- Signal handlers ---- */

static void
on_entry_activate (GtkEntry *ent, gpointer data)
{
  const char *text = gtk_entry_get_text (ent);
  if (text[0] == '\0') return;

  /* Echo command */
  append_text ("> ", "prompt");
  append_text (text, "command");
  append_text ("\n", NULL);

  /* Send to algae */
  send_command (text);

  /* Add to history */
  add_to_history (text);

  /* Clear entry */
  gtk_entry_set_text (ent, "");
}

static gboolean
on_entry_key_press (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  if (event->keyval == GDK_KEY_Up)
    {
      if (history_pos > 0)
        {
          history_pos--;
          gtk_entry_set_text (GTK_ENTRY (entry), cmd_history[history_pos]);
          gtk_editable_set_position (GTK_EDITABLE (entry), -1);
        }
      return TRUE;
    }
  else if (event->keyval == GDK_KEY_Down)
    {
      if (history_pos < history_count - 1)
        {
          history_pos++;
          gtk_entry_set_text (GTK_ENTRY (entry), cmd_history[history_pos]);
          gtk_editable_set_position (GTK_EDITABLE (entry), -1);
        }
      else
        {
          history_pos = history_count;
          gtk_entry_set_text (GTK_ENTRY (entry), "");
        }
      return TRUE;
    }
  return FALSE;
}

/* ---- Menu callbacks ---- */

static void
on_open_file (GtkMenuItem *item, gpointer data)
{
  GtkWidget *dialog = gtk_file_chooser_dialog_new (
    "Open Algae File", GTK_WINDOW (window),
    GTK_FILE_CHOOSER_ACTION_OPEN,
    "_Cancel", GTK_RESPONSE_CANCEL,
    "_Open", GTK_RESPONSE_ACCEPT,
    NULL);

  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "Algae files (*.a)");
  gtk_file_filter_add_pattern (filter, "*.a");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, "All files");
  gtk_file_filter_add_pattern (filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      char cmd[2048];
      snprintf (cmd, sizeof(cmd), "source(\"%s\")", filename);
      append_text ("> ", "prompt");
      append_text (cmd, "command");
      append_text ("\n", NULL);
      send_command (cmd);
      g_free (filename);
    }

  gtk_widget_destroy (dialog);
}

static void
on_clear (GtkMenuItem *item, gpointer data)
{
  gtk_text_buffer_set_text (text_buffer, "", 0);
}

static void
on_quit (GtkMenuItem *item, gpointer data)
{
  stop_algae_process ();
  gtk_main_quit ();
}

static void
on_about (GtkMenuItem *item, gpointer data)
{
  GtkWidget *dialog = gtk_about_dialog_new ();
  gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialog), "Algae");
  gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), "4.3.6");
  gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (dialog),
    "A numerical analysis programming language.\n"
    "GTK+ interface for interactive use.");
  gtk_about_dialog_set_license_type (GTK_ABOUT_DIALOG (dialog),
                                      GTK_LICENSE_GPL_2_0);
  gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog),
    "https://github.com/navoj/Algae");
  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void
on_help_contents (GtkMenuItem *item, gpointer data)
{
  send_command ("help()");
}

static void
on_restart (GtkMenuItem *item, gpointer data)
{
  stop_algae_process ();
  append_text ("\n--- Restarting Algae ---\n\n", "info");
  start_algae_process ();
}

static void
on_window_destroy (GtkWidget *widget, gpointer data)
{
  stop_algae_process ();
  gtk_main_quit ();
}

/* ---- Build the GUI ---- */

static GtkWidget *
create_menu_bar (void)
{
  GtkWidget *menubar = gtk_menu_bar_new ();

  /* File menu */
  {
    GtkWidget *file_menu = gtk_menu_new ();
    GtkWidget *file_item = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_item), file_menu);

    GtkWidget *open = gtk_menu_item_new_with_mnemonic ("_Open...");
    g_signal_connect (open, "activate", G_CALLBACK (on_open_file), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), open);

    GtkWidget *clear = gtk_menu_item_new_with_mnemonic ("_Clear Console");
    g_signal_connect (clear, "activate", G_CALLBACK (on_clear), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), clear);

    gtk_menu_shell_append (GTK_MENU_SHELL (file_menu),
                           gtk_separator_menu_item_new ());

    GtkWidget *restart = gtk_menu_item_new_with_mnemonic ("_Restart Algae");
    g_signal_connect (restart, "activate", G_CALLBACK (on_restart), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), restart);

    gtk_menu_shell_append (GTK_MENU_SHELL (file_menu),
                           gtk_separator_menu_item_new ());

    GtkWidget *quit = gtk_menu_item_new_with_mnemonic ("_Quit");
    g_signal_connect (quit, "activate", G_CALLBACK (on_quit), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), quit);

    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), file_item);
  }

  /* Help menu */
  {
    GtkWidget *help_menu = gtk_menu_new ();
    GtkWidget *help_item = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_item), help_menu);

    GtkWidget *contents = gtk_menu_item_new_with_mnemonic ("_Builtin Functions");
    g_signal_connect (contents, "activate", G_CALLBACK (on_help_contents), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (help_menu), contents);

    gtk_menu_shell_append (GTK_MENU_SHELL (help_menu),
                           gtk_separator_menu_item_new ());

    GtkWidget *about = gtk_menu_item_new_with_mnemonic ("_About");
    g_signal_connect (about, "activate", G_CALLBACK (on_about), NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (help_menu), about);

    gtk_menu_shell_append (GTK_MENU_SHELL (menubar), help_item);
  }

  return menubar;
}

static void
setup_tags (GtkTextBuffer *buffer)
{
  gtk_text_buffer_create_tag (buffer, "output",
    "foreground", "#000000",
    "font", "Monospace 10",
    NULL);

  gtk_text_buffer_create_tag (buffer, "error",
    "foreground", "#CC0000",
    "font", "Monospace 10",
    "weight", PANGO_WEIGHT_BOLD,
    NULL);

  gtk_text_buffer_create_tag (buffer, "info",
    "foreground", "#0066CC",
    "font", "Monospace 10",
    "style", PANGO_STYLE_ITALIC,
    NULL);

  gtk_text_buffer_create_tag (buffer, "prompt",
    "foreground", "#006600",
    "font", "Monospace 10 Bold",
    NULL);

  gtk_text_buffer_create_tag (buffer, "command",
    "foreground", "#000099",
    "font", "Monospace 10 Bold",
    NULL);
}

int
main (int argc, char *argv[])
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *scroll;
  GtkWidget *label;
  GtkWidget *menubar;

  gtk_init (&argc, &argv);

  /* Main window */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Algae 4.3.6");
  gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
  g_signal_connect (window, "destroy", G_CALLBACK (on_window_destroy), NULL);

  /* Main layout */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* Menu bar */
  menubar = create_menu_bar ();
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 0);

  /* Text view (console output) */
  text_view = gtk_text_view_new ();
  text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (text_view), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_CHAR);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 4);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 4);
  setup_tags (text_buffer);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scroll), text_view);
  gtk_box_pack_start (GTK_BOX (vbox), scroll, TRUE, TRUE, 0);

  /* Input area */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2);

  label = gtk_label_new (" > ");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_placeholder_text (GTK_ENTRY (entry),
    "Enter Algae command...");
  g_signal_connect (entry, "activate",
                    G_CALLBACK (on_entry_activate), NULL);
  g_signal_connect (entry, "key-press-event",
                    G_CALLBACK (on_entry_key_press), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 0);

  /* Status bar */
  statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), statusbar, FALSE, FALSE, 0);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 0, "Ready");

  /* Show everything */
  gtk_widget_show_all (window);

  /* Welcome message */
  append_text ("Algae 4.3.6 -- GTK+ Interface\n", "info");
  append_text ("Type commands below or use File > Open to source a file.\n\n",
               "info");

  /* Start the Algae backend */
  start_algae_process ();

  /* Focus the entry */
  gtk_widget_grab_focus (entry);

  /* Run GTK main loop */
  gtk_main ();

  /* Cleanup */
  {
    int i;
    for (i = 0; i < history_count; i++)
      g_free (cmd_history[i]);
  }

  return 0;
}

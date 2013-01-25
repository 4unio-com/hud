extern const string HUD_GTK_DATADIR;

namespace HudGtk {
	public class CellRendererVariant : Gtk.CellRendererText {
		public Variant value {
			set {
				if (value != null) {
					text = value.print (false);
				} else {
					text = "(null)";
				}
			}
		}
	}

	class Window : Gtk.ApplicationWindow {
		Gtk.ListStore model;
		HudClient.Query query;
		
		void results_row_added (Dee.Model results, Dee.ModelIter result_iter) {
			var pos = results.get_position(result_iter);

			Gtk.TreeIter iter;
			model.insert(out iter, (int)pos);

			model.set(iter, 0, results.get_string(result_iter, 1)); /* Command */
			model.set(iter, 1, results.get_string(result_iter, 3)); /* Description */
			model.set(iter, 2, results.get_string(result_iter, 5)); /* Shortcut */
			model.set(iter, 3, results.get_string(result_iter, 6)); /* Distance */
			model.set(iter, 4, results.get_value(result_iter, 0)); /* Key */
		}

		void results_row_removed (Dee.Model results, Dee.ModelIter result_iter) {
			var pos = results.get_position(result_iter);

			string spath = "%d";
			spath = spath.printf(pos);
			Gtk.TreePath path = new Gtk.TreePath.from_string(spath);
			Gtk.TreeIter iter;
			model.get_iter(out iter, path);
			model.remove(iter);
		}

		void entry_text_changed (Object object, ParamSpec pspec) {
			var entry = object as Gtk.Entry;
			query.set_query(entry.text);
		}

		void voice_pressed (Gtk.Button button) {
			query.voice_query();
		}

		void view_activated (Gtk.TreeView view, Gtk.TreePath path, Gtk.TreeViewColumn column) {
			Gtk.TreeIter iter;
			Variant key;

			model.get_iter (out iter, path);
			model.get (iter, 4, out key);

			query.execute_command(key, 0);
		}

		public Window (Gtk.Application application) {
			Object (application: application, title: "Hud");
			set_default_size (500, 300);

			var builder = new Gtk.Builder ();
			try {
				new CellRendererVariant ();
				builder.add_from_file (HUD_GTK_DATADIR + "/hud-gtk.ui");
				query = new HudClient.Query("");
			} catch (Error e) {
				error (e.message);
			}

			Dee.Model results = query.get_results_model();
			results.row_added.connect (results_row_added);
			results.row_removed.connect (results_row_removed);

			model = builder.get_object ("liststore") as Gtk.ListStore;
			builder.get_object ("entry").notify["text"].connect (entry_text_changed);
			(builder.get_object ("voice") as Gtk.Button).clicked.connect (voice_pressed);
			(builder.get_object ("treeview") as Gtk.TreeView).row_activated.connect (view_activated);
			add (builder.get_object ("grid") as Gtk.Widget);
		}
	}

	class Application : Gtk.Application {
		protected override void activate () {
			new Window (this).show_all ();
		}

		public Application () {
			Object (application_id: "com.canonical.HudGtk");
		}
	}
}

int main (string[] args) {
	return new HudGtk.Application ().run (args);
}

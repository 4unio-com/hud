extern void gtk_widget_insert_action_group (Gtk.Widget widget, string name, ActionGroup group);

namespace GVTk {
	public Gtk.Widget create (Variant items, ActionGroup? actions) {
		var grid = new Gtk.Grid ();
		gtk_widget_insert_action_group (grid, "op", actions);

		foreach (var item in items) {
			var type = item.lookup_value ("type", VariantType.STRING);

			if (type == null) {
				continue;
			}

			Gtk.Widget left;
			Gtk.Widget right;

			switch (type.get_string ()) {
				case "slider":
					create_slider (out left, out right, item, actions);
					break;
				case "button":
					create_button (out left, out right, item, actions);
					break;
				case "toggle":
					create_toggle (out left, out right, item, actions);
					break;
				default:
					continue;
			}

			grid.attach_next_to (right, null, Gtk.PositionType.BOTTOM, 1, 1);
			if (left != null)
				grid.attach_next_to (left, right, Gtk.PositionType.LEFT, 1, 1);
		}

		grid.show_all ();

		return grid;
	}

	Gtk.Widget create_label (Variant item) {
		var label = item.lookup_value ("label", VariantType.STRING);

		return new Gtk.Label (label != null ? label.get_string () : "");
	}

	void create_slider (out Gtk.Widget left, out Gtk.Widget right, Variant item, ActionGroup? actions) {
		var range = item.lookup_value ("range", new VariantType ("(**)"));
		var action = item.lookup_value ("action", VariantType.STRING);
		right = new Gtk.Scale (Gtk.Orientation.HORIZONTAL, null);
		left = create_label (item);
		right.set_hexpand (true);
	}

	void create_button (out Gtk.Widget left, out Gtk.Widget right, Variant item, ActionGroup? actions) {
		var action = item.lookup_value ("action", VariantType.STRING);
		var label = item.lookup_value ("label", VariantType.STRING);
		right = new Gtk.Button.with_label (label != null ? label.get_string () : "");
		(right as Gtk.Button).set_action_name (action.get_string ());
		left = null;
	}

	void create_toggle (out Gtk.Widget left, out Gtk.Widget right, Variant item, ActionGroup? actions) {
		var action = item.lookup_value ("action", VariantType.STRING);
		right = new Gtk.CheckButton ();
		left = create_label (item);
		(right as Gtk.Button).set_action_name (action.get_string ());
		right.set_hexpand (true);
	}
}

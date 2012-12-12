public class Waiter {
	bool done;

	bool cb () {
		done = true;
		return false;
	}

	public Waiter (uint ms) {
		Timeout.add (ms, cb);
		while (!done) {
			((MainContext) null).iteration (true);
		}
	}
}

const string operation_one = "[{'type': <'slider'>, 'action': <'op.radius'>, 'label': <'Radius'>, 'range': <(0.0, 100.0)>}, {'type': <'toggle'>, 'action': <'op.gaussian'>, 'label': <'Gaussian'>}, {'type': <'button'>, 'action': <'op.apply'>, 'label': <'Apply'>}]";

const string operation_two = "[{'type': <'slider'>, 'action': <'op.red'>, 'label': <'Red'>, 'range': <(0, 255)>}, {'type': <'slider'>, 'action': <'op.green'>, 'label': <'Green'>, 'range': <(0, 255)>}, {'type': <'slider'>, 'action': <'op.blue'>, 'label': <'Blue'>, 'range': <(0, 255)>}, {'type': <'button'>, 'action': <'op.apply'>, 'label': <'Apply'>}]";

void main (string[] args) {
	Gtk.init (ref args);

	var bus = Bus.get_sync (BusType.SESSION, null);
	var group = DBusActionGroup.get (bus, args[1], "/org/gtk/Application/anonymous");
	group.has_action ("foo");

	new Waiter (100);

	var window = new Gtk.Window ();
	var yodawg = YoDawgIHeardYouLikeActionGroup.start (group, "blur", null);
	window.add (GVTk.create (Variant.parse (null, operation_one, null, null), yodawg));
	window.show_all ();

	yodawg = null;

	Gtk.main ();
}

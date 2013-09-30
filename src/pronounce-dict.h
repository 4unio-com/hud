#ifndef __PRONOUNCE_DICT_H__
#define __PRONOUNCE_DICT_H__

#include <glib-object.h>

G_BEGIN_DECLS

#define PRONOUNCE_DICT_TYPE            (pronounce_dict_get_type ())
#define PRONOUNCE_DICT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), PRONOUNCE_DICT_TYPE, PronounceDict))
#define PRONOUNCE_DICT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PRONOUNCE_DICT_TYPE, PronounceDictClass))
#define IS_PRONOUNCE_DICT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PRONOUNCE_DICT_TYPE))
#define IS_PRONOUNCE_DICT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PRONOUNCE_DICT_TYPE))
#define PRONOUNCE_DICT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PRONOUNCE_DICT_TYPE, PronounceDictClass))

typedef struct _PronounceDict         PronounceDict;
typedef struct _PronounceDictClass    PronounceDictClass;
typedef struct _PronounceDictPrivate  PronounceDictPrivate;

struct _PronounceDictClass {
	GObjectClass parent_class;

};

struct _PronounceDict {
	GObject parent;

	PronounceDictPrivate * priv;
};

GType pronounce_dict_get_type (void);
gchar ** pronounce_dict_lookup_word(PronounceDict * dict, gchar * word);
PronounceDict * pronounce_dict_new (const gchar *dict_path, GError **error);
PronounceDict * pronounce_dict_get_sphinx (GError **error);

G_END_DECLS

#endif

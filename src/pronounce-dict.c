#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pronounce-dict.h"

struct _PronounceDictPrivate {
	GHashTable * dict;
};

#define PRONOUNCE_DICT_GET_PRIVATE(o) \
(G_TYPE_INSTANCE_GET_PRIVATE ((o), PRONOUNCE_DICT_TYPE, PronounceDictPrivate))

static void pronounce_dict_class_init (PronounceDictClass *klass);
static void pronounce_dict_init       (PronounceDict *self);
static void pronounce_dict_dispose    (GObject *object);
static void pronounce_dict_finalize   (GObject *object);

G_DEFINE_TYPE (PronounceDict, pronounce_dict, G_TYPE_OBJECT);

static void
pronounce_dict_class_init (PronounceDictClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (PronounceDictPrivate));

	object_class->dispose = pronounce_dict_dispose;
	object_class->finalize = pronounce_dict_finalize;

	return;
}

static void
pronounce_dict_init (PronounceDict *self)
{
	self->priv = PRONOUNCE_DICT_GET_PRIVATE(self);

	return;
}

static void
pronounce_dict_dispose (GObject *object)
{

	G_OBJECT_CLASS (pronounce_dict_parent_class)->dispose (object);
	return;
}

static void
pronounce_dict_finalize (GObject *object)
{

	G_OBJECT_CLASS (pronounce_dict_parent_class)->finalize (object);
	return;
}

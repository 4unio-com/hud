
#ifndef __YO_DAWG_I_HEARD_YOU_LIKE_ACTION_GROUP_H__
#define __YO_DAWG_I_HEARD_YOU_LIKE_ACTION_GROUP_H__

#include <gio/gio.h>

GActionGroup *          yo_dawg_i_heard_you_like_action_group_start     (GActionGroup *parent,
                                                                         const gchar  *action_name,
                                                                         GVariant     *setup_data);

#endif /* __YO_DAWG_I_HEARD_YOU_LIKE_ACTION_GROUP_H__ */

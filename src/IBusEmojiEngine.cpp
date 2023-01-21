#if __has_include("ibus.h")

#include "IBusEmojiEngine.hpp"
#include "logging.hpp"
#include "EmojiWindow.hpp"
#include <cctype>
#include <memory>
#include <string>

extern "C" {
typedef struct _IBusEmojiEngine IBusEmojiEngine;
typedef struct _IBusEmojiEngineClass IBusEmojiEngineClass;

struct _IBusEmojiEngine {
  IBusEngine parent;
};

struct _IBusEmojiEngineClass {
  IBusEngineClass parent;
};

static void ibus_emoji_engine_class_init(IBusEmojiEngineClass* emoji_engine_class);

static void ibus_emoji_engine_init(IBusEmojiEngine* emoji_engine);
static void ibus_emoji_engine_destroy(IBusEmojiEngine* emoji_engine);

static void ibus_emoji_engine_enable(IBusEngine* engine);
static void ibus_emoji_engine_disable(IBusEngine* engine);
static void ibus_emoji_engine_reset(IBusEngine* engine);
static gboolean ibus_emoji_engine_process_key_event(IBusEngine* engine, guint keyval, guint keycode, guint modifiers);
static void ibus_emoji_engine_set_cursor_location(IBusEngine* engine, gint x, gint y, gint w, gint h);
// static void ibus_emoji_engine_focus_in(IBusEngine* engine);
// static void ibus_emoji_engine_focus_out(IBusEngine* engine);
// static void ibus_emoji_engine_set_capabilities(IBusEngine* engine, guint caps);
// static void ibus_emoji_engine_set_content_type(IBusEngine* engine, guint purpose, guint hint);
// static void ibus_emoji_engine_set_surrounding_text(IBusEngine* engine, IBusText* text, guint cursor_index, guint anchor_pos);
// static void ibus_emoji_engine_page_up(IBusEngine* engine);
// static void ibus_emoji_engine_page_down(IBusEngine* engine);
// static void ibus_emoji_engine_cursor_up(IBusEngine* engine);
// static void ibus_emoji_engine_cursor_down(IBusEngine* engine);
// static void ibus_emoji_engine_candidate_clicked(IBusEngine* engine, guint index, guint button, guint state);
// static void ibus_emoji_engine_property_activate(IBusEngine* engine, const gchar* prop_name, guint prop_state);
// static void ibus_emoji_engine_property_show(IBusEngine* engine, const gchar* prop_name);
// static void ibus_emoji_engine_property_hide(IBusEngine* engine, const gchar* prop_name);

G_DEFINE_TYPE(IBusEmojiEngine, ibus_emoji_engine, IBUS_TYPE_ENGINE)

static void ibus_emoji_engine_class_init(IBusEmojiEngineClass* emoji_engine_class) {
  log_printf("[debug] ibus_emoji_engine_class_init\n");

  IBusObjectClass* ibus_object_class = IBUS_OBJECT_CLASS(emoji_engine_class);
  ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_emoji_engine_destroy;

  IBusEngineClass* ibus_engine_class = IBUS_ENGINE_CLASS(emoji_engine_class);
  ibus_engine_class->enable = ibus_emoji_engine_enable;
  ibus_engine_class->disable = ibus_emoji_engine_disable;
  ibus_engine_class->reset = ibus_emoji_engine_reset;
  ibus_engine_class->process_key_event = ibus_emoji_engine_process_key_event;
  ibus_engine_class->set_cursor_location = ibus_emoji_engine_set_cursor_location;
  ibus_engine_class->focus_in = nullptr; // ibus_emoji_engine_focus_in;
  ibus_engine_class->focus_out = nullptr; // ibus_emoji_engine_focus_out;
  ibus_engine_class->set_capabilities = nullptr; // ibus_emoji_engine_set_capabilities;
  ibus_engine_class->set_content_type = nullptr; // ibus_emoji_engine_set_content_type;
  ibus_engine_class->set_surrounding_text = nullptr; // ibus_emoji_engine_set_surrounding_text;
  ibus_engine_class->page_up = nullptr; // ibus_emoji_engine_page_up;
  ibus_engine_class->page_down = nullptr; // ibus_emoji_engine_page_down;
  ibus_engine_class->cursor_up = nullptr; // ibus_emoji_engine_cursor_up;
  ibus_engine_class->cursor_down = nullptr; // ibus_emoji_engine_cursor_down;
  ibus_engine_class->candidate_clicked = nullptr; // ibus_emoji_engine_candidate_clicked;
  ibus_engine_class->property_activate = nullptr; // ibus_emoji_engine_property_activate;
  ibus_engine_class->property_show = nullptr; // ibus_emoji_engine_property_show;
  ibus_engine_class->property_hide = nullptr; // ibus_emoji_engine_property_hide;
}

static void ibus_emoji_engine_init(IBusEmojiEngine* emoji_engine) {
  log_printf("[debug] ibus_emoji_engine_init\n");
}

static void ibus_emoji_engine_destroy(IBusEmojiEngine* emoji_engine) {
  log_printf("[debug] ibus_emoji_engine_destroy\n");

  ((IBusObjectClass*)ibus_emoji_engine_parent_class)->destroy((IBusObject*)emoji_engine);
}

static void ibus_emoji_engine_enable(IBusEngine* engine) {
  log_printf("[debug] ibus_emoji_engine_enable\n");

  emoji_command_queue.push(std::make_shared<EmojiCommandEnable>([engine](const std::string& text) {
    ibus_engine_commit_text(engine, ibus_text_new_from_string(text.data()));
  }));
}

static void ibus_emoji_engine_disable(IBusEngine* engine) {
  log_printf("[debug] ibus_emoji_engine_disable\n");

  emoji_command_queue.push(std::make_shared<EmojiCommandDisable>());
}

static void ibus_emoji_engine_reset(IBusEngine* engine) {
  log_printf("[debug] ibus_emoji_engine_reset\n");

  emoji_command_queue.push(std::make_shared<EmojiCommandReset>());
}

#define KEYCODE_ESCAPE 1
#define KEYCODE_RETURN 28
#define KEYCODE_BACKSPACE 14
#define KEYCODE_ARROW_UP 103
#define KEYCODE_ARROW_DOWN 108
#define KEYCODE_ARROW_LEFT 105
#define KEYCODE_ARROW_RIGHT 106

static gboolean ibus_emoji_engine_process_key_event(IBusEngine* engine, guint keyval, guint keycode, guint modifiers) {
  log_printf("[debug] ibus_emoji_engine_process_key_event keyval:%c keycode:%d modifiers:%d\n", keyval, keycode, modifiers);

  QKeyEvent::Type _type = (modifiers & IBUS_RELEASE_MASK) ? QKeyEvent::KeyRelease : QKeyEvent::KeyPress;
  int _key = 0;
  switch (keycode) {
  case KEYCODE_ESCAPE: // IBUS_KEY_Escape:
    _key = Qt::Key_Escape;
    break;
  case KEYCODE_RETURN: // IBUS_KEY_Return:
    _key = Qt::Key_Return;
    break;
  case KEYCODE_BACKSPACE: // IBUS_KEY_BackSpace:
    _key = Qt::Key_Backspace;
    break;
  case KEYCODE_ARROW_UP: // IBUS_KEY_uparrow:
    _key = Qt::Key_Up;
    break;
  case KEYCODE_ARROW_DOWN: // IBUS_KEY_downarrow:
    _key = Qt::Key_Down;
    break;
  case KEYCODE_ARROW_LEFT: // IBUS_KEY_leftarrow:
    _key = Qt::Key_Left;
    break;
  case KEYCODE_ARROW_RIGHT: // IBUS_KEY_rightarrow:
    _key = Qt::Key_Right;
    break;
  }
  Qt::KeyboardModifiers _modifiers = Qt::NoModifier;
  if (modifiers & IBUS_SUPER_MASK) {
    return FALSE;
  }
  if (modifiers & IBUS_CONTROL_MASK) {
    _modifiers = _modifiers | Qt::ControlModifier;
  }
  if (modifiers & IBUS_SHIFT_MASK) {
    _modifiers = _modifiers | Qt::ShiftModifier;
  }
  QString _text = QString::fromStdString(std::string{(char)keyval});

  if (_key != 0 || (_text.length() == 1 && isascii(_text.at(0).toLatin1()))) {
    emoji_command_queue.push(std::make_shared<EmojiCommandProcessKeyEvent>(new QKeyEvent(_type, _key, _modifiers, _text)));

    return TRUE;
  }

  return FALSE;
}

static void ibus_emoji_engine_set_cursor_location(IBusEngine* engine, gint x, gint y, gint w, gint h) {
  log_printf("[debug] ibus_emoji_engine_set_cursor_location x:%d y:%d w:%d h:%d\n", x, y, w, h);

  emoji_command_queue.push(std::make_shared<EmojiCommandSetCursorLocation>(new QRect(x, y, w, h)));
}
}

#endif

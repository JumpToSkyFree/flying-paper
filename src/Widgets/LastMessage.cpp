#include "Widgets/LastMessage.hpp"
#include "Telegram/ClientManager.hpp"
#include "Telegram/Internal/ClientManagerAccessor.hpp"
#include "Telegram/Session.hpp"
#include "peel/GLib/functions.h"
#include "peel/Gtk/BinLayout.h"
#include "peel/Gtk/Image.h"
#include "peel/Gtk/Label.h"
#include "peel/Gtk/Orientation.h"
#include "peel/Gtk/Picture.h"
#include "peel/Gtk/Widget.h"
#include "peel/Pango/EllipsizeMode.h"
#include <memory>
#include <peel/ArrayRef.h>
#include <peel/FloatPtr.h>
#include <peel/GObject/Object.h>
#include <peel/RefPtr.h>
#include <peel/class.h>
#include <td/telegram/td_api.h>
#include <td/tl/TlObject.h>

namespace FlyingPaper::Widgets {
PEEL_CLASS_IMPL(LastMessage, "FlyingPaperLastMessage", Gtk::Widget);
void LastMessage::Class::init() {
  set_layout_manager_type(::Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<LastMessage>();
}
inline void LastMessage::vfunc_dispose() {
  if (container) {
    container->unparent();
    container = nullptr;
  }
  parent_vfunc_dispose<LastMessage>();
}
inline void LastMessage::init(Class *) {
  FloatPtr<Gtk::Box> _container =
      Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 8);
  container = _container;

  text_content = Gtk::Label::create("");
  text_content->set_single_line_mode(true);
  text_content->set_ellipsize(Pango::EllipsizeMode::END);

  image_content = Gtk::Image::create();
  image_content->set_pixel_size(20);
  image_content->add_css_class("chat-list-item-picture");

  _container->append(image_content);
  _container->append(text_content);
  set_parent(std::move(_container));
}
RefPtr<LastMessage> LastMessage::create() {
  return ::Object::create<LastMessage>();
}
void LastMessage::set_label_with_markup(bool read, const std::string &text) {
  if (read) {
    String markup = "<span alpha='50%%'>%s</span>";
    markup = GLib::strdup_printf(markup, text.c_str());
    text_content->set_markup(markup.c_str());
  } else {
    String markup = "<b>%s</b>";
    markup = GLib::strdup_printf(markup, text.c_str());
    text_content->set_markup(markup.c_str());
  }
  this->read = read;
}
void LastMessage::set_message_content(
    bool read,
    td::td_api::object_ptr<td::td_api::MessageContent> message_content) {
  if (message_content) {
    bool is_message_content_set = false;
    auto _message_content =
        Telegram::ClientManager::cast_ptr<td::td_api::MessageContent,
                                          td::td_api::MessageContent>(
            std::move(message_content));
    Telegram::ClientManagerAccessor::handle<td::td_api::MessageContent,
                                            td::td_api::messageText>(
        _message_content,
        [this, read, &is_message_content_set](
            const std::shared_ptr<td::td_api::messageText> &text) {
          image_content->set_visible(false);
          text_content->set_visible(true);
          set_label_with_markup(read, text->text_->text_);
          is_message_content_set = true;
        },
        nullptr);
    Telegram::ClientManagerAccessor::handle<td::td_api::MessageContent,
                                            td::td_api::messageSticker>(
        _message_content,
        [this, read, &is_message_content_set](
            const std::shared_ptr<td::td_api::messageSticker> &sticker) {
          text_content->set_visible(true);
          if (sticker->sticker_) {
            image_content->set_visible(true);
            auto session = Session::Session::get();
            auto client = session->get_client();
            std::string photo = "Sticker";
            set_label_with_markup(read, photo);

            // FIX: Gtk doesn't load webp formats by default.

            client->download_file(
                sticker->sticker_->sticker_, 1,
                [this](const td::td_api::file &file) {
                  RefPtr<Gtk::Picture> picture =
                      Gtk::Picture::create_for_filename(
                          file.local_->path_.c_str());
                  image_content->set_from_paintable(picture->get_paintable());
                },
                nullptr);
            is_message_content_set = true;
          }
        },
        nullptr);
    Telegram::ClientManagerAccessor::handle<td::td_api::MessageContent,
                                            td::td_api::messagePhoto>(
        _message_content,
        [this, read, &is_message_content_set](
            const std::shared_ptr<td::td_api::messagePhoto> &photo) {
          text_content->set_visible(true);
          if (photo) {
            if (photo->caption_->text_.length()) {
              set_label_with_markup(read, photo->caption_->text_);
            } else {
              std::string photo = "Photo";
              set_label_with_markup(read, photo);
            }
            if (photo->photo_) {
              image_content->set_visible(true);
              auto session = Session::Session::get();
              auto client = session->get_client();
              if (photo->photo_->sizes_.size() && photo->photo_->sizes_[0]) {
                client->download_file(
                    photo->photo_->sizes_[0]->photo_, 1,
                    [this](const td::td_api::file &file) {
                      RefPtr<Gtk::Picture> picture =
                          Gtk::Picture::create_for_filename(
                              file.local_->path_.c_str());
                      image_content->set_from_paintable(
                          picture->get_paintable());
                    },
                    nullptr);
                is_message_content_set = true;
              }
            }
          }
        },
        nullptr);
    Telegram::ClientManagerAccessor::handle<td::td_api::MessageContent,
                                            td::td_api::messageVideo>(
        _message_content,
        [this, read, &is_message_content_set](
            const std::shared_ptr<td::td_api::messageVideo> &video) {
          text_content->set_visible(true);
          if (video) {
            if (video->caption_->text_.length()) {
              set_label_with_markup(read, video->caption_->text_);
            } else {
              std::string photo = "Photo";
              set_label_with_markup(read, photo);
            }
            if (video->video_) {
              image_content->set_visible(true);
              auto session = Session::Session::get();
              auto client = session->get_client();
              if (video->video_->thumbnail_ &&
                  video->video_->thumbnail_->file_) {
                client->download_file(
                    video->video_->thumbnail_->file_, 1,
                    [this](const td::td_api::file &file) {
                      RefPtr<Gtk::Picture> picture =
                          Gtk::Picture::create_for_filename(
                              file.local_->path_.c_str());
                      image_content->set_from_paintable(
                          picture->get_paintable());
                    },
                    nullptr);
                is_message_content_set = true;
              }
            }
          }
        },
        nullptr);
    Telegram::ClientManagerAccessor::handle<td::td_api::MessageContent,
                                            td::td_api::messageAnimation>(
        _message_content,
        [this, read, &is_message_content_set](
            const std::shared_ptr<td::td_api::messageAnimation> &gif) {
          text_content->set_visible(true);
          if (gif) {
            if (gif->caption_->text_.length()) {
              set_label_with_markup(read, gif->caption_->text_);
            } else {
              std::string photo = "GIF";
              set_label_with_markup(read, photo);
            }
            if (gif->animation_) {
              image_content->set_visible(true);
              auto session = Session::Session::get();
              auto client = session->get_client();
              if (gif->animation_->thumbnail_ &&
                  gif->animation_->thumbnail_->file_) {
                client->download_file(
                    gif->animation_->thumbnail_->file_, 1,
                    [this](const td::td_api::file &file) {
                      RefPtr<Gtk::Picture> picture =
                          Gtk::Picture::create_for_filename(
                              file.local_->path_.c_str());
                      image_content->set_from_paintable(
                          picture->get_paintable());
                    },
                    nullptr);
                is_message_content_set = true;
              }
            }
          }
        },
        nullptr);
    if (!is_message_content_set) {
      image_content->set_visible(false);
      text_content->set_visible(true);
      set_label_with_markup(true, "Message content not supported yet.");
      is_message_content_set = true;
    }
  }
}
void LastMessage::set_read(bool read) {
  set_label_with_markup(read, text_content->get_text());
  this->read = read;
}
} // namespace FlyingPaper::Widgets

#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>
#include <format>

#include "../../core/include/paket.h"

namespace fs = std::filesystem;

enum struct State {
  Init,
  PreEncrypting,
  PreDecrypting,
};

struct PaketCtx {
  State state;
  std::string key;
  fs::path path;
  Fl_Widget* input;
  Fl_Widget* button;
};

void process_cb(Fl_Widget* widget, void* userdata) {
  PaketCtx* ctx = (PaketCtx*)userdata;
  Fl_Input* input = (Fl_Input*)ctx->input;

  const std::string key = input->value();

  if (key.length() < 8) {
    fl_alert("Key must be at least 8 characters long");
    return;
  }

  Fl_Native_File_Chooser chooser;

  switch (ctx->state) {
    case State::PreDecrypting: {
      std::string output_path;

      chooser.title("Enter dir for decrypted files to appear");
      chooser.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
      chooser.options(Fl_Native_File_Chooser::NEW_FOLDER | Fl_Native_File_Chooser::SAVEAS_CONFIRM);

      switch (chooser.show()) {
        case -1:
        case 1:
          return;
        case 0:
          output_path = chooser.filename();
      }

      PaketRes res = decrypt(
        ctx->path.string(),
        output_path,
        key
      );

      switch (res) {
        case PaketRes::Ok: {
          fl_message("Done decrypting!");
          return;
        }
        case PaketRes::NotValidHeader: {
          fl_alert("Not valid file specified for decrypting, try file that ends with .pkt.");
          return;
        }
        case PaketRes::WrongKey: {
          fl_alert("Try other key, this seems to be wrong.");
        }
        default:
          return;
      };

      break;
    }
    case State::PreEncrypting: {
      std::string encrypted_filename = ctx->path.stem().string() + ".pkt";
      fs::path output_file;

      chooser.title("Enter ecnrypted file");
      chooser.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
      chooser.options(Fl_Native_File_Chooser::NEW_FOLDER | Fl_Native_File_Chooser::SAVEAS_CONFIRM);
      chooser.filter("Paket file\t*.pkt\n");

      switch (chooser.show()) {
        case -1:
        case 1:
          return;
        case 0:
          output_file = chooser.filename();
      }

      output_file.replace_extension(".pkt");

      PaketRes res = encrypt(
        ctx->path.string(),
        output_file.string(),
        key
      );

      switch (res) {
        case PaketRes::Ok: {
          fl_message("Done encrypting!");
          return;
        }

        default: {
          fl_alert("Something when wrong...");
          return;
        }
      }

      break;
    }
    default:
      return;
  }
}

// Custom box to handle drag and drop
class DragDropBox : public Fl_Box {
  PaketCtx* ctx;

public:
  DragDropBox(
    PaketCtx* ctx,
    int X,
    int Y,
    int W,
    int H,
    const char* L = 0
  ) : Fl_Box(X, Y, W, H, L) {
    this->ctx = ctx;

    box(FL_DOWN_BOX);
    color(FL_LIGHT2);
    labelsize(14);
    labelfont(FL_BOLD);
    label("Drag & Drop Files Here");
    align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
  }

  int handle(int event) {
    switch(event) {
      case FL_DND_ENTER:
      case FL_DND_DRAG:
        return 1;
      case FL_DND_RELEASE:
        return 1;
      case FL_PASTE: {
        const char* dropped_files = Fl::event_text();
        std::vector<std::string> files = this->parseDroppedFiles(dropped_files);
        this->displayDroppedFiles(files);
        return 1;
      }
      default:
        return Fl_Box::handle(event);
    }
  }

private:
  std::vector<std::string> parseDroppedFiles(const char* dropped_files) {
    std::vector<std::string> files;
    std::stringstream ss(dropped_files);
    std::string file;
    while (std::getline(ss, file, '\n')) {
      files.push_back(file);
    }
    return files;
  }

  void displayDroppedFiles(const std::vector<std::string>& files) {
    const fs::path path = fs::path(files.back());

    copy_label(path.filename().string().c_str());
    redraw();

    this->ctx->path = path;
    this->ctx->state = path.extension().string() == ".pkt" ? State::PreDecrypting : State::PreEncrypting;

    this->ctx->button->label(
      this->ctx->state == State::PreDecrypting ? "Decrypt": "Encrypt"
    );
    this->ctx->button->redraw();
  }
};

int main() {
  PaketCtx ctx;

  const int width = 250;
  const int height = 325;
  const int spacing = 10;

  Fl_Window* window = new Fl_Window(width, height, "Paket");

  Fl_Pack *pack = new Fl_Pack(
    spacing,
    spacing * 2,
    width - spacing * 2,
    height - spacing * 2
  );
  pack->type(Fl_Pack::VERTICAL);
  pack->spacing(spacing);

  const int actionsHeight = 30;

  // Textbox at the top
  Fl_Input* input = new Fl_Input(
    spacing,
    0,
    width - spacing * 2,
    actionsHeight,
    "Key"
  );
  input->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
  ctx.input = input;

  // Drag and Drop area
  new DragDropBox(
    &ctx,
    spacing,
    0,
    width - spacing * 2,
    height - spacing * 5 - actionsHeight * 2
  );

  // Encrypt button at the bottom
  Fl_Button* encrypt_button = new Fl_Button(
    spacing,
    0,
    width - spacing * 2,
    actionsHeight,
    "Drop files"
  );
  encrypt_button->callback(process_cb, (void*)&ctx);
  ctx.button = encrypt_button;

  pack->end();
  window->end();
  window->show();

  return Fl::run();
}

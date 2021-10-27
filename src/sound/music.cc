#include "music.h"
#include "../load_from_file_worker.h"

namespace node_sfml {
namespace sound {

using v8::Boolean;
using v8::Function;
using v8::FunctionTemplate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

namespace {
Nan::Persistent<Function> constructor;
}

NAN_MODULE_INIT(Music::Init) {
  Local<String> name = Nan::New("Music").ToLocalChecked();

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);

  Nan::SetPrototypeMethod(tpl, "setLoop", SetLoop);
  Nan::SetPrototypeMethod(tpl, "getLoop", GetLoop);

  Nan::SetPrototypeMethod(tpl, "openFromFileSync", OpenFromFileSync);
  Nan::SetPrototypeMethod(tpl, "openFromFile", OpenFromFile);

  sound::SoundSource::SetCommonPrototype(&tpl);

  tpl->SetClassName(name);
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Local<Function> func = Nan::GetFunction(tpl).ToLocalChecked();
  constructor.Reset(func);

  Nan::Set(target, name, func);
}

NAN_METHOD(Music::New) {
  Music* Texture = new Music();
  Texture->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

inline bool OpenFromFileFunction(void* target,
                                 const std::string& filename,
                                 void* _) {
  return static_cast<Music*>(target)->music().openFromFile(filename);
}

NAN_METHOD(Music::SetLoop) {
  Music* music = Nan::ObjectWrap::Unwrap<Music>(info.Holder());
  bool val = Nan::To<bool>(info[0]).FromJust();
  music->music().setLoop(val);
}

NAN_METHOD(Music::GetLoop) {
  Music* music = Nan::ObjectWrap::Unwrap<Music>(info.Holder());
  bool val = music->music().getLoop();
  Local<Boolean> ret = Nan::New<Boolean>(val);
  info.GetReturnValue().Set(ret);
}

NAN_METHOD(Music::OpenFromFile) {
  Music* music = Nan::ObjectWrap::Unwrap<Music>(info.Holder());
  if (music->_loading) {
    Local<Value> err = Nan::Error("Music is loading.");
    info.GetReturnValue().Set(err);

    return;
  }

  music->_loading = true;
  Local<String> v8_filename = info[0].As<String>();
  Nan::Utf8String utf8_filename(v8_filename);
  Nan::Callback* callback = new Nan::Callback(info[1].As<Function>());

  load_from_file_worker::LoadFromFileWorker<Music, void>* worker =
      new load_from_file_worker::LoadFromFileWorker<Music, void>(
          info.Holder(),
          *utf8_filename,
          OpenFromFileFunction,
          nullptr,
          callback);
  Nan::AsyncQueueWorker(worker);
}

NAN_METHOD(Music::OpenFromFileSync) {
  Music* music = Nan::ObjectWrap::Unwrap<Music>(info.Holder());
  Nan::Utf8String filename(info[0].As<String>());
  info.GetReturnValue().Set(music->music().openFromFile(*filename));
}

Music::Music() : SoundSource(new sf::Music()) {}
Music::~Music() {
  music().stop();
}

}  // namespace sound
}  // namespace node_sfml

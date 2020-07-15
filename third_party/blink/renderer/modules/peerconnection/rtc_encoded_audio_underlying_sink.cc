// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/peerconnection/rtc_encoded_audio_underlying_sink.h"

#include "third_party/blink/renderer/bindings/modules/v8/v8_rtc_encoded_audio_frame.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/peerconnection/rtc_encoded_audio_stream_transformer.h"
#include "third_party/webrtc/api/frame_transformer_interface.h"

namespace blink {

RTCEncodedAudioUnderlyingSink::RTCEncodedAudioUnderlyingSink(
    ScriptState* script_state,
    TransformerCallback transformer_callback)
    : transformer_callback_(std::move(transformer_callback)) {
  DCHECK(transformer_callback_);
}

ScriptPromise RTCEncodedAudioUnderlyingSink::start(
    ScriptState* script_state,
    WritableStreamDefaultController* controller,
    ExceptionState&) {
  // No extra setup needed.
  return ScriptPromise::CastUndefined(script_state);
}

ScriptPromise RTCEncodedAudioUnderlyingSink::write(
    ScriptState* script_state,
    ScriptValue chunk,
    WritableStreamDefaultController* controller,
    ExceptionState& exception_state) {
  RTCEncodedAudioFrame* encoded_frame =
      V8RTCEncodedAudioFrame::ToImplWithTypeCheck(script_state->GetIsolate(),
                                                  chunk.V8Value());
  if (!encoded_frame) {
    exception_state.ThrowDOMException(DOMExceptionCode::kTypeMismatchError,
                                      "Invalid frame");
    return ScriptPromise();
  }

  // Get webrtc frame and send it to the decoder.
  if (!transformer_callback_) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Stream closed");
    return ScriptPromise();
  }

  transformer_callback_.Run()->SendFrameToSink(
      encoded_frame->PassWebRtcFrame());
  return ScriptPromise::CastUndefined(script_state);
}

ScriptPromise RTCEncodedAudioUnderlyingSink::close(ScriptState* script_state,
                                                   ExceptionState&) {
  // Disconnect from the transformer if the sink is closed.
  if (transformer_callback_)
    transformer_callback_.Reset();
  return ScriptPromise::CastUndefined(script_state);
}

ScriptPromise RTCEncodedAudioUnderlyingSink::abort(
    ScriptState* script_state,
    ScriptValue reason,
    ExceptionState& exception_state) {
  // It is not possible to cancel any frames already sent to the WebRTC sink,
  // thus abort() has the same effect as close().
  return close(script_state, exception_state);
}

void RTCEncodedAudioUnderlyingSink::Trace(Visitor* visitor) {
  UnderlyingSinkBase::Trace(visitor);
}

}  // namespace blink

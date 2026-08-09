#include "SerialNetworkInterface.h"

void SerialNetworkInterface::begin(int port) {
  server.begin(port);
}

void SerialNetworkInterface::enable() {
  if (_isEnabled) return;

  _isEnabled = true;
  clearBuffers();
}

void SerialNetworkInterface::disable() {
  _isEnabled = false;
}

size_t SerialNetworkInterface::writeFrame(const uint8_t src[], size_t len) {
  if (len > MAX_FRAME_SIZE) {
    NETWORK_DEBUG_PRINTLN("writeFrame(), frame too big, len=%d", len);
    return 0;
  }

  if (deviceConnected && len > 0) {
    if (send_queue_len >= FRAME_QUEUE_SIZE) {
      NETWORK_DEBUG_PRINTLN("writeFrame(), send_queue is full!");
      return 0;
    }

    send_queue[send_queue_len].len = len;
    memcpy(send_queue[send_queue_len].buf, src, len);
    send_queue_len++;

    return len;
  }
  return 0;
}

bool SerialNetworkInterface::isWriteBusy() const {
  return false;
}

bool SerialNetworkInterface::hasReceivedFrameHeader() {
  return received_frame_header.type != 0 && received_frame_header.length != 0;
}

void SerialNetworkInterface::resetReceivedFrameHeader() {
  received_frame_header.type = 0;
  received_frame_header.length = 0;
}

size_t SerialNetworkInterface::checkRecvFrame(uint8_t dest[]) {
  auto newClient = server.accept();
  if (newClient) {
    deviceConnected = false;
    client.stop();
    client = newClient;
    resetReceivedFrameHeader();
  }

  if (client.connected()) {
    if (!deviceConnected) {
      NETWORK_DEBUG_PRINTLN("Got connection");
      deviceConnected = true;
    }
  } else if (deviceConnected) {
    deviceConnected = false;
    NETWORK_DEBUG_PRINTLN("Disconnected");
  }

  if (!deviceConnected) {
    return 0;
  }

  if (send_queue_len > 0) {
    _last_write = millis();
    int len = send_queue[0].len;

    uint8_t pkt[3 + len];
    pkt[0] = '>';
    pkt[1] = (len & 0xFF);
    pkt[2] = (len >> 8);
    memcpy(&pkt[3], send_queue[0].buf, send_queue[0].len);
    client.write(pkt, 3 + len);
    send_queue_len--;
    for (int i = 0; i < send_queue_len; i++) {
      send_queue[i] = send_queue[i + 1];
    }
    return 0;
  }

  if (!hasReceivedFrameHeader()) {
    const int frame_header_length = 3;
    if (client.available() >= frame_header_length) {
      client.readBytes(&received_frame_header.type, 1);
      client.readBytes((uint8_t *)&received_frame_header.length, 2);
    }
  }

  if (!hasReceivedFrameHeader()) {
    return 0;
  }

  int available = client.available();
  int frame_type = received_frame_header.type;
  int frame_length = received_frame_header.length;
  if (frame_length > available) {
    NETWORK_DEBUG_PRINTLN("Waiting for %d more bytes", frame_length - available);
    return 0;
  }

  if (frame_length > MAX_FRAME_SIZE) {
    NETWORK_DEBUG_PRINTLN("Skipping frame: length=%d is larger than MAX_FRAME_SIZE=%d", frame_length, MAX_FRAME_SIZE);
    while (frame_length > 0) {
      uint8_t skip[1];
      int skipped = client.read(skip, 1);
      frame_length -= skipped;
    }
    resetReceivedFrameHeader();
    return 0;
  }

  if (frame_type != '<') {
    NETWORK_DEBUG_PRINTLN("Skipping frame: type=0x%x is unexpected", frame_type);
    while (frame_length > 0) {
      uint8_t skip[1];
      int skipped = client.read(skip, 1);
      frame_length -= skipped;
    }
    resetReceivedFrameHeader();
    return 0;
  }

  client.readBytes(dest, frame_length);
  resetReceivedFrameHeader();
  return frame_length;
}

bool SerialNetworkInterface::isConnected() const {
  return deviceConnected;
}

#ifndef COMMS_H
#define COMMS_H

#define COMMS_BAUD_RATE 115200

#define PROTOCOL_MSGS_INIT         '#'
#define PROTOCOL_MSGS_END          '/'
#define PROTOCOL_PARAMS_SEPARATOR  ':'

#define PROTOCOL_MSG_FORMAT "#%c:%s/"

#define PROTOCOL_MSG_SIZE 26
#define ULONG_DIGITS 10
#define N_DECIMALS 2

enum msg_id {
  START_LAP = 0,
  END_LAP,
  OBSTACLE_DETECTED,
  LINE_LOST,
  PING,
  INIT_LINE_SEARCH,
  STOP_LINE_SEARCH,
  LINE_FOUND,
  VISIBLE_LINE,
  INVALID_MSG,
  ACK
};

// Protocol --> #id:arg/#id:arg/...
struct protocol_msg {
  int id;
  float arg;
};

String create_msg_with_protocol(struct protocol_msg* msg);
struct protocol_msg read_msg(Stream *serial_port);
struct protocol_msg msg_to_protocol(String msg);

void trim_spaces(char* str) {
  char* start = str;
  while (*start == ' ') {
    start++;
  }

  if (start != str) {
    memmove(str, start, strlen(start) + 1);
  }
}

// Cambiar por punteros
String create_msg_with_protocol(struct protocol_msg* msg) {
  return PROTOCOL_MSGS_INIT +
          String(msg->id) +
          PROTOCOL_PARAMS_SEPARATOR +
          String(msg->arg, N_DECIMALS) +
          PROTOCOL_MSGS_END;
}

struct protocol_msg read_msg(Stream *serial_port) {
  struct protocol_msg msg;
  char readed_byte;
  String msg_buffer = "";
  bool is_reading = false, dots_readed = false;

  while (serial_port->available() > 0 || is_reading) {    
    readed_byte = serial_port->read();

    if (readed_byte == PROTOCOL_MSGS_INIT) {
      if (!is_reading) {
        is_reading = true;
        continue;
      } else {
        break;
      }
    }

    if (is_reading) {
      if (readed_byte != PROTOCOL_MSGS_END) {
        msg_buffer += String(readed_byte);

        if (readed_byte == PROTOCOL_PARAMS_SEPARATOR) {
          if (!dots_readed) {
            dots_readed = true;
          } else {
            break;
          }
        }

      } else {
        if (!dots_readed) {
          break;
        }

        Serial.println(msg_buffer);

        return msg_to_protocol(msg_buffer);
      }
    }
  }

  msg.id = INVALID_MSG;

  return msg;
}

struct protocol_msg msg_to_protocol(String msg) {
  String id_str, arg_str;
  int separator_idx;
  struct protocol_msg return_msg;

  separator_idx = msg.indexOf(PROTOCOL_PARAMS_SEPARATOR);

  id_str = msg.substring(0, separator_idx);
  arg_str = msg.substring(separator_idx + 1, msg.length());

  return_msg.id = msg_id(id_str.toInt());
  return_msg.arg = arg_str.toFloat();

  return return_msg;
}

#endif // COMMS_H

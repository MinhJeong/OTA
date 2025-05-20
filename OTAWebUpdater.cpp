#include "OTAWebUpdater.h"

OTAWebUpdater::OTAWebUpdater(WebServer *server, const char* host) {
  this->server = server;
  this->host = host;
  setupStyle();
  setupServerPage();
}

void OTAWebUpdater::setup(const char* username, const char* password) {
  this->loginUsername = username;
  this->loginPassword = password;
  setupLoginPage();
}

void OTAWebUpdater::setupStyle() {
  this->style =
    "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
    "input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
    "#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
    "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
    "form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
    ".btn{background:#3498db;color:#fff;cursor:pointer}</style>";
}

void OTAWebUpdater::setupLoginPage() {
  this->loginIndex =
    "<form name=loginForm>"
    "<h1>ESP32 Login</h1>"
    "<input name=userid placeholder='User ID'> "
    "<input name=pwd placeholder=Password type=Password> "
    "<input type=submit onclick=check(this.form) class=btn value=Login></form>"
    "<script>"
    "function check(form) {"
    "if(form.userid.value=='" + this->loginUsername + "' && form.pwd.value=='" + this->loginPassword + "')"
    "{window.open('/serverIndex')}"
    "else"
    "{alert('Error Password or Username')}"
    "}"
    "</script>";
}

void OTAWebUpdater::setupServerPage() {
  this->serverIndex = 
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<h1>ESP32 Update Firmware</h1>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
    "<label id='file-input' for='file'>   Chọn file...</label>"
    "<input type='submit' class=btn value='Cập nhật'>"
    "<br><br>"
    "<div id='prg'>0%</div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('Tiến trình: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('Thành công!');"
    "alert('Cập nhật thành công! Thiết bị sẽ khởi động lại.');"
    "},"
    "error: function (a, b, c) {"
    "alert('Lỗi cập nhật! Vui lòng thử lại.');"
    "}"
    "});"
    "});"
    "</script>";
}

void OTAWebUpdater::handleLogin() {
  String page = this->style + this->loginIndex;
  this->server->send(200, "text/html", page);
}

void OTAWebUpdater::handleServerIndex() {
  String page = this->style + this->serverIndex;
  this->server->send(200, "text/html", page);
}

void OTAWebUpdater::handleUpdate() {
  this->server->sendHeader("Connection", "close");
  this->server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}

void OTAWebUpdater::registerEndpoints() {
  // Đăng ký các route
  this->server->on("/", HTTP_GET, [this]() {
    this->handleLogin();
  });
  
  this->server->on("/serverIndex", HTTP_GET, [this]() {
    this->handleServerIndex();
  });
  
  // Xử lý khi nhận firmware
  this->server->on("/update", HTTP_POST, 
    [this]() {
      this->handleUpdate();
    },
    [this]() {
      HTTPUpload& upload = this->server->upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
          Serial.printf("Update Success: %u\nRestarting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.end();
        Serial.println("Update aborted");
      }
    }
  );
}
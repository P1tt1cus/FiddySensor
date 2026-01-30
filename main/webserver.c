#include "webserver.h"
#include "moisture_sensor.h"
#include <stdio.h>
#include <string.h>

static const char *HTML =
    "<!DOCTYPE html><html><head>"
    "<title>Fiddy Leaf</title>"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
    "<style>"
    "*{margin:0;padding:0;box-sizing:border-box}"
    "body{font-family:system-ui,sans-serif;min-height:100vh;display:flex;flex-direction:column;"
    "align-items:center;justify-content:center;background:#0a0a0a;padding:20px;color:#fff}"
    ".card{text-align:center;margin-bottom:40px}"
    ".name{font-size:14px;color:#444;margin-bottom:20px;letter-spacing:4px;text-transform:uppercase}"
    ".plant{position:relative;width:100px;height:120px;margin:0 auto 25px}"
    ".pot{position:absolute;bottom:0;left:50%;transform:translateX(-50%);width:40px;height:28px;"
    "background:#4a3728;border-radius:2px 2px 8px 8px}"
    ".pot::before{content:'';position:absolute;top:-5px;left:-4px;width:48px;height:7px;"
    "background:#5c4433;border-radius:2px}"
    ".stem{position:absolute;bottom:28px;left:50%;width:4px;height:50px;"
    "background:#2d5a2d;transform:translateX(-50%);border-radius:2px}"
    ".leaf{position:absolute;width:24px;height:36px;border-radius:50% 50% 50% 50%/60% 60% 40% 40%;"
    "transform-origin:bottom center;background:#3da33d}"
    ".l1{bottom:65px;left:18px;transform:rotate(-20deg);animation:sway 4s ease-in-out infinite}"
    ".l2{bottom:72px;left:50%;transform:translateX(-50%);animation:sway 4.5s ease-in-out infinite .5s}"
    ".l3{bottom:65px;right:18px;transform:rotate(20deg);animation:sway 3.8s ease-in-out infinite 1s}"
    ".l4{bottom:50px;left:24px;transform:rotate(-30deg) scale(0.8);animation:sway 4.2s ease-in-out infinite .3s}"
    ".l5{bottom:50px;right:24px;transform:rotate(30deg) scale(0.8);animation:sway 3.6s ease-in-out infinite .8s}"
    "@keyframes sway{0%,100%{rotate:var(--r,0deg)}50%{rotate:calc(var(--r,0deg) + 5deg)}}"
    ".l1{--r:-20deg}.l2{--r:0deg}.l3{--r:20deg}.l4{--r:-30deg}.l5{--r:30deg}"
    ".moist .leaf{background:#6b8e23}.moist .stem{background:#4a6b23}"
    ".dry .leaf{background:#c4a035}.dry .stem{background:#7a6b23}"
    ".ded .leaf{background:#4a4a4a;animation:none}.ded .stem{background:#3a3a3a}"
    ".ded .l1,.ded .l4{transform:rotate(-60deg) scale(0.9)}.ded .l3,.ded .l5{transform:rotate(60deg) scale(0.9)}"
    ".ded .l2{transform:translateX(-50%) scaleY(0.8)}"
    ".pct{font-size:64px;font-weight:100;color:#fff;line-height:1}"
    ".status{margin-top:20px}"
    ".msg{font-size:13px;font-weight:500;padding:8px 16px;border-radius:20px;display:inline-block;"
    "background:#1a1a1a;border:1px solid #333}"
    ".wet .msg{border-color:#3da33d;color:#3da33d}"
    ".moist .msg{border-color:#6b8e23;color:#6b8e23}"
    ".dry .msg{border-color:#c4a035;color:#c4a035}"
    ".ded .msg{border-color:#666;color:#666}"
    ".graph-box{background:#111;border-radius:8px;padding:16px;width:100%;max-width:400px;border:1px solid #222}"
    ".graph-title{color:#444;font-size:10px;margin-bottom:8px;letter-spacing:2px;text-transform:uppercase}"
    "canvas{width:100%;height:120px;display:block}"
    "</style></head>"
    "<body>"
    "<div class=\"card\" id=\"card\">"
    "<div class=\"name\">Fiddy Leaf</div>"
    "<div class=\"plant\" id=\"plant\">"
    "<div class=\"leaf l1\"></div><div class=\"leaf l2\"></div><div class=\"leaf l3\"></div>"
    "<div class=\"leaf l4\"></div><div class=\"leaf l5\"></div>"
    "<div class=\"stem\"></div><div class=\"pot\"></div>"
    "</div>"
    "<div class=\"pct\" id=\"p\">--%</div>"
    "<div class=\"status\"><div class=\"msg\" id=\"m\">...</div></div>"
    "</div>"
    "<div class=\"graph-box\">"
    "<div class=\"graph-title\"><span id=\"cnt\">0</span> readings over <span id=\"span\">-</span></div>"
    "<canvas id=\"g\"></canvas>"
    "</div>"
    "<script>"
    "const msgs={"
    "wet:['THOU HAST BLESSED ME WITH HYDRATION','I BATHE IN THY HOLY WATER','I LIVETH','DROWNING IN BLESSINGS','WET AND THRIVING BESTIE','SPLASH SPLASH I WAS TAKIN A BATH','MOISTURE LEVEL: THICC','HYDRATION STATION ACTIVATED','FEELING CUTE MIGHT PHOTOSYNTHESIZE LATER','THE SOIL IS BUSSIN FR FR','LIVING MY BEST CHLOROPHYLL LIFE','DRIP CHECK: PASSED','CERTIFIED MOIST BOI','WATER YOU WAITING FOR? JK IM GOOD','ALEXA PLAY WATERFALL BY TLC','MAIN CHARACTER ENERGY RN','MOISTURIZED UNBOTHERED IN MY LANE'],"
    "moist:['THE END TIMES DRAW NEAR','MINE ROOTS GROW WEARY','I SENSE A GREAT DROUGHT','GETTING CRUSTY NGL','SOS SENT','LOWKEY STRESSED RN','VIBES ARE DETERIORATING','ITS GIVING... CONCERNING','MY THERAPIST WILL HEAR ABOUT THIS','ENTERING MY VILLAIN ERA','POV: U FORGOT ME AGAIN','THIS IS A CRY FOR HELP BTW','BESTIE CHECK ON ME PLS','IM IN MY FLOP ERA','NOT TO BE DRAMATIC BUT','HYDRATION WARRANTY EXPIRING SOON','THE AUDACITY OF THIS NEGLECT'],"
    "dry:['HAVE MERCY UPON THY PLANT','I BEG THEE, BRING WATER','LORD GIVE ME STRENGTH','THIS IS NOT A DRILL','HELP ME STEP-GARDENER','I CANT EVEN RN','WHY HAST THOU GHOSTED ME','WATER ME OR SQUARE UP','911 ID LIKE TO REPORT A CRIME','MOM COME PICK ME UP IM SCARED','NOT ME DYING IN 4K','HELLO DARKNESS MY OLD FRIEND','IM LITERALLY CRUNCHING','TOUCH GRASS? I AM THE GRASS','BRUH','THIS IS YOUR FAULT SPECIFICALLY','DO I MEAN NOTHING TO YOU','GOOGLE HOW TO SUE YOUR OWNER'],"
    "ded:['FATHER WHY HAST THOU FORSAKEN ME','BURY ME WHENCE I CAME','TELL MY SEEDS I LOVED THEM','THIS IS MY 13TH REASON','GOODBYE CRUEL WORLD','I HOPE UR HAPPY KAREN','GG NO RE','PRESS F TO PAY RESPECTS','RIP BOZO (ITS ME IM BOZO)','SKILL ISSUE TBH','SHOULD HAVE BOUGHT A FAKE PLANT','REST IN PEPPERONI','I LIVED I LOVED I WILTED','GONE BUT FORGOTTEN','COMPOSTING MY WAY TO HEAVEN','MY ROMAN EMPIRE IS OVER','DESERVED BETTER NGL']"
    "};"
    "const pick=a=>a[Math.floor(Math.random()*a.length)];"
    "let last='';"
    "const canvas=document.getElementById('g');"
    "const ctx=canvas.getContext('2d');"
    "function fmtTime(s){if(s<60)return s+'s';if(s<3600)return Math.floor(s/60)+'m';if(s<86400)return Math.floor(s/3600)+'h';return Math.floor(s/86400)+'d';}"
    "function drawGraph(data,uptime){"
    "const w=canvas.width=canvas.offsetWidth*2;"
    "const h=canvas.height=canvas.offsetHeight*2;"
    "ctx.clearRect(0,0,w,h);"
    "if(data.length<2)return;"
    "const t0=data[0].t,t1=uptime,span=t1-t0||1;"
    "document.getElementById('span').textContent=fmtTime(span);"
    "ctx.strokeStyle='#222';ctx.lineWidth=1;"
    "for(let i=0;i<=4;i++){const y=h*i/4;ctx.beginPath();ctx.moveTo(0,y);ctx.lineTo(w,y);ctx.stroke();}"
    "ctx.fillStyle='#333';ctx.font='18px system-ui';ctx.fillText('0%',5,h-5);ctx.fillText('100%',5,22);"
    "ctx.beginPath();ctx.strokeStyle='#3da33d';ctx.lineWidth=2;"
    "let lastX=0;"
    "data.forEach((pt,i)=>{const x=((pt.t-t0)/span)*w;const y=h-(pt.p/100)*h;lastX=x;i===0?ctx.moveTo(x,y):ctx.lineTo(x,y);});"
    "ctx.stroke();"
    "const grad=ctx.createLinearGradient(0,0,0,h);"
    "grad.addColorStop(0,'rgba(61,163,61,0.2)');grad.addColorStop(1,'rgba(61,163,61,0)');"
    "ctx.lineTo(lastX,h);ctx.lineTo(0,h);ctx.closePath();ctx.fillStyle=grad;ctx.fill();"
    "}"
    "function update(){"
    "fetch('/data').then(r=>r.json()).then(d=>{"
    "document.getElementById('p').textContent=d.percent+'%';"
    "const m=document.getElementById('m');"
    "const c=document.getElementById('card');"
    "const p=document.getElementById('plant');"
    "let cls=d.percent>70?'wet':d.percent>40?'moist':d.percent>20?'dry':'ded';"
    "if(cls!==last){m.textContent=pick(msgs[cls]);last=cls;}"
    "c.className='card '+cls;"
    "p.className='plant '+cls;"
    "});"
    "fetch('/history').then(r=>r.json()).then(d=>{"
    "document.getElementById('cnt').textContent=d.count;"
    "drawGraph(d.data,d.uptime);"
    "});"
    "}"
    "setInterval(update,5000);"
    "update();"
    "</script></body></html>";

static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    httpd_resp_send(req, HTML, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t data_handler(httpd_req_t *req)
{
    char json[64];
    snprintf(json, sizeof(json), "{\"raw\":%d,\"percent\":%d}",
             moisture_sensor_get_raw(),
             moisture_sensor_get_percent());
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t history_handler(httpd_req_t *req)
{
    static history_entry_t buf[HISTORY_SIZE];
    int count = moisture_sensor_get_history(buf, HISTORY_SIZE);
    uint32_t uptime = moisture_sensor_get_uptime();

    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr_chunk(req, "{\"count\":");

    char num[32];
    snprintf(num, sizeof(num), "%d", count);
    httpd_resp_sendstr_chunk(req, num);

    snprintf(num, sizeof(num), ",\"uptime\":%lu,\"data\":[", (unsigned long)uptime);
    httpd_resp_sendstr_chunk(req, num);

    for (int i = 0; i < count; i++) {
        snprintf(num, sizeof(num), "%s{\"t\":%lu,\"p\":%d}",
                 i > 0 ? "," : "",
                 (unsigned long)buf[i].timestamp,
                 buf[i].percent);
        httpd_resp_sendstr_chunk(req, num);
    }

    httpd_resp_sendstr_chunk(req, "]}");
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
}

httpd_handle_t webserver_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root = {.uri = "/", .method = HTTP_GET, .handler = root_handler};
        httpd_uri_t data = {.uri = "/data", .method = HTTP_GET, .handler = data_handler};
        httpd_uri_t history = {.uri = "/history", .method = HTTP_GET, .handler = history_handler};
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &data);
        httpd_register_uri_handler(server, &history);
    }
    return server;
}

void webserver_stop(httpd_handle_t server)
{
    if (server) httpd_stop(server);
}

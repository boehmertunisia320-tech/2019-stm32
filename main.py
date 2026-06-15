from maix import camera, display, image, touchscreen, uart, time

# ========= 初始化 =========
# 修改分辨率为 320x240
cam = camera.Camera(320, 240)
disp = display.Display()
ts = touchscreen.TouchScreen()
ser = uart.UART("/dev/ttyS0", 115200)

# ========= 模式 =========
mode = 1  # 0=调参, 1=识别

# ========= LAB =========
Lmin=5
Lmax=100
Amin=20
Amax=127
Bmin=0
Bmax=20

param = 0
names = ["Lmin","Lmax","Amin","Amax","Bmin","Bmax"]

def clamp_pair(a, b, minv, maxv):
    if a > b:
        b = a
    a = max(minv, a)
    b = min(maxv, b)
    return a, b

last_touch = 0

def map_coord(x, y):
    return x // 2, y // 2

# ========= FPS =========
fps_cnt = 0
fps = 0
last_fps = time.ticks_ms()

# ========= 主循环 =========
while True:

    img = cam.read()

    # ================= FPS =================
    fps_cnt += 1
    now = time.ticks_ms()

    if now - last_fps >= 1000:
        fps = fps_cnt
        fps_cnt = 0
        last_fps = now

    # ================= 触摸 =================
    t = ts.read()

    if t and isinstance(t, list) and len(t) >= 3:
        # 映射触摸坐标到 320x240
        x, y = map_coord(t[0], t[1])
        press = t[2]

        if press and now - last_touch > 200:
            last_touch = now

            # ===== 切换模式按钮 =====
            if 210 < x < 310 and 10 < y < 40:
                mode = 1 - mode   # 0↔1

            # ===== 调参模式 =====
            if mode == 0:
                # [-] 按钮
                if 10 < x < 50 and 10 < y < 35:
                    if param==0: Lmin=max(0,Lmin-1)
                    elif param==1: Lmax=max(0,Lmax-1)
                    elif param==2: Amin=max(-128,Amin-1)
                    elif param==3: Amax=max(-128,Amax-1)
                    elif param==4: Bmin=max(-128,Bmin-1)
                    elif param==5: Bmax=max(-128,Bmax-1)

                # [+] 按钮
                elif 60 < x < 100 and 10 < y < 35:
                    if param==0: Lmin=min(100,Lmin+1)
                    elif param==1: Lmax=min(100,Lmax+1)
                    elif param==2: Amin=min(127,Amin+1)
                    elif param==3: Amax=min(127,Amax+1)
                    elif param==4: Bmin=min(127,Bmin+1)
                    elif param==5: Bmax=min(127,Bmax+1)

                # 切换参数按钮
                elif 110 < x < 200 and 10 < y < 35:
                    param = (param + 1) % 6

                Lmin, Lmax = clamp_pair(Lmin, Lmax, 0, 100)
                Amin, Amax = clamp_pair(Amin, Amax, -128, 127)
                Bmin, Bmax = clamp_pair(Bmin, Bmax, -128, 127)

    # ================= 模式0：调参 =================
    if mode == 0:

        th = (Lmin,Lmax,Amin,Amax,Bmin,Bmax)

        blobs = img.find_blobs(
            [th],
            pixels_threshold=50,
            area_threshold=50,
            merge=True
        )

        if blobs:
            b = max(blobs, key=lambda x:x.w()*x.h())
            img.draw_rect(b.x(), b.y(), b.w(), b.h(), image.COLOR_RED)

        # UI 按钮绘制
        img.draw_rect(10,10,40,25,image.COLOR_WHITE)
        img.draw_string(22,15,"-",image.COLOR_WHITE)

        img.draw_rect(60,10,40,25,image.COLOR_WHITE)
        img.draw_string(72,15,"+",image.COLOR_WHITE)

        img.draw_rect(110,10,90,25,image.COLOR_WHITE)
        img.draw_string(117,15,names[param],image.COLOR_RED)

        img.draw_rect(210,10,100,30,image.COLOR_GREEN)
        img.draw_string(215,15,"MODE->RUN",image.COLOR_GREEN)

        # 参数显示位置
        img.draw_string(5,50,f"L:{Lmin}-{Lmax}",image.COLOR_WHITE)
        img.draw_string(5,65,f"A:{Amin}-{Amax}",image.COLOR_WHITE)
        img.draw_string(5,80,f"B:{Bmin}-{Bmax}",image.COLOR_WHITE)

    # ================= 模式1：识别 =================
    else:

        red_thresh = (Lmin,Lmax,Amin,Amax,Bmin,Bmax)

        blobs = img.find_blobs(
            [red_thresh],
            pixels_threshold=100,
            area_threshold=100,
            merge=True
        )

        if blobs:
            b = max(blobs, key=lambda x:x.w()*x.h())
            cx = b.x() + b.w()//2
            cy = b.y() + b.h()//2
            r = max(b.w(), b.h()) // 2

            img.draw_circle(cx, cy, r, image.COLOR_RED, 2)
            img.draw_cross(cx, cy, image.COLOR_GREEN)

            ser.write(bytes([
                ord('['),
                (cx>>8)&0xff,
                cx&0xff,
                (cy>>8)&0xff,
                cy&0xff,
                ord(']')
            ]))
        else:
            ser.write(b'[\xfe11]')

        # 显示识别结果和 FPS
        if blobs:
            text = f"X:{cx} Y:{cy}"
        else:
            text = "X:-1 Y:-1"

        img.draw_string(5, 10, text, image.COLOR_WHITE)        
        img.draw_string(5, 20, f"FPS:{fps}", image.COLOR_WHITE)

        img.draw_rect(210,10,100,30,image.COLOR_RED)
        img.draw_string(215,15,"MODE->LAB",image.COLOR_RED)

    # ================= 显示 =================
    disp.show(img)
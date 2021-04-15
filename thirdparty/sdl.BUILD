licenses(["permissive"])

cc_library(
    name = "SDL",
    srcs = [
        "src/SDL.c",
        "src/SDL_assert.c",
        "src/SDL_error.c",
        "src/SDL_hints.c",
        "src/SDL_log.c",
        "src/atomic/SDL_atomic.c",
        "src/atomic/SDL_spinlock.c",
        "src/audio/SDL_audio.c",
        "src/audio/SDL_audiocvt.c",
        "src/audio/SDL_audiodev.c",
        "src/audio/SDL_audiotypecvt.c",
        "src/audio/SDL_mixer.c",
        "src/audio/SDL_wave.c",
        "src/core/linux/SDL_dbus.c",
        "src/core/linux/SDL_evdev.c",
        "src/core/linux/SDL_fcitx.c",
        "src/core/linux/SDL_ibus.c",
        "src/core/linux/SDL_ime.c",
        "src/core/linux/SDL_udev.c",
        "src/cpuinfo/SDL_cpuinfo.c",
        "src/dynapi/SDL_dynapi.c",
        "src/events/SDL_clipboardevents.c",
        "src/events/SDL_dropevents.c",
        "src/events/SDL_events.c",
        "src/events/SDL_gesture.c",
        "src/events/SDL_keyboard.c",
        "src/events/SDL_mouse.c",
        "src/events/SDL_quit.c",
        "src/events/SDL_touch.c",
        "src/events/SDL_windowevents.c",
        "src/file/SDL_rwops.c",
        "src/filesystem/unix/SDL_sysfilesystem.c",
        "src/haptic/dummy/SDL_syshaptic.c",
        "src/joystick/SDL_gamecontroller.c",
        "src/joystick/SDL_joystick.c",
        "src/joystick/linux/SDL_sysjoystick.c",
        "src/libm/e_atan2.c",
        "src/libm/e_log.c",
        "src/libm/e_pow.c",
        "src/libm/e_rem_pio2.c",
        "src/libm/e_sqrt.c",
        "src/libm/k_cos.c",
        "src/libm/k_rem_pio2.c",
        "src/libm/k_sin.c",
        "src/libm/k_tan.c",
        "src/libm/s_atan.c",
        "src/libm/s_copysign.c",
        "src/libm/s_cos.c",
        "src/libm/s_fabs.c",
        "src/libm/s_floor.c",
        "src/libm/s_scalbn.c",
        "src/libm/s_sin.c",
        "src/libm/s_tan.c",
        "src/loadso/dlopen/SDL_sysloadso.c",
        "src/power/SDL_power.c",
        "src/power/linux/SDL_syspower.c",
        "src/render/SDL_d3dmath.c",
        "src/render/SDL_render.c",
        "src/render/SDL_yuv_mmx.c",
        "src/render/SDL_yuv_sw.c",
        "src/render/direct3d/SDL_render_d3d.c",
        "src/render/direct3d11/SDL_render_d3d11.c",
        "src/render/opengl/SDL_render_gl.c",
        "src/render/opengl/SDL_shaders_gl.c",
        "src/render/opengles/SDL_render_gles.c",
        "src/render/opengles2/SDL_render_gles2.c",
        "src/render/opengles2/SDL_shaders_gles2.c",
        "src/render/psp/SDL_render_psp.c",
        "src/render/software/SDL_blendfillrect.c",
        "src/render/software/SDL_blendline.c",
        "src/render/software/SDL_blendpoint.c",
        "src/render/software/SDL_drawline.c",
        "src/render/software/SDL_drawpoint.c",
        "src/render/software/SDL_render_sw.c",
        "src/render/software/SDL_rotate.c",
        "src/stdlib/SDL_getenv.c",
        "src/stdlib/SDL_iconv.c",
        "src/stdlib/SDL_malloc.c",
        "src/stdlib/SDL_qsort.c",
        "src/stdlib/SDL_stdlib.c",
        "src/stdlib/SDL_string.c",
        "src/thread/SDL_thread.c",
        "src/thread/pthread/SDL_syscond.c",
        "src/thread/pthread/SDL_sysmutex.c",
        "src/thread/pthread/SDL_syssem.c",
        "src/thread/pthread/SDL_systhread.c",
        "src/thread/pthread/SDL_systls.c",
        "src/timer/SDL_timer.c",
        "src/timer/unix/SDL_systimer.c",
        "src/video/SDL_RLEaccel.c",
        "src/video/SDL_blit.c",
        "src/video/SDL_blit_0.c",
        "src/video/SDL_blit_1.c",
        "src/video/SDL_blit_A.c",
        "src/video/SDL_blit_N.c",
        "src/video/SDL_blit_auto.c",
        "src/video/SDL_blit_copy.c",
        "src/video/SDL_blit_slow.c",
        "src/video/SDL_bmp.c",
        "src/video/SDL_clipboard.c",
        "src/video/SDL_egl.c",
        "src/video/SDL_fillrect.c",
        "src/video/SDL_pixels.c",
        "src/video/SDL_rect.c",
        "src/video/SDL_shape.c",
        "src/video/SDL_stretch.c",
        "src/video/SDL_surface.c",
        "src/video/SDL_video.c",
        "src/video/x11/SDL_x11clipboard.c",
        "src/video/x11/SDL_x11dyn.c",
        "src/video/x11/SDL_x11events.c",
        "src/video/x11/SDL_x11framebuffer.c",
        "src/video/x11/SDL_x11keyboard.c",
        "src/video/x11/SDL_x11messagebox.c",
        "src/video/x11/SDL_x11modes.c",
        "src/video/x11/SDL_x11mouse.c",
        "src/video/x11/SDL_x11opengl.c",
        "src/video/x11/SDL_x11opengles.c",
        "src/video/x11/SDL_x11shape.c",
        "src/video/x11/SDL_x11touch.c",
        "src/video/x11/SDL_x11video.c",
        "src/video/x11/SDL_x11window.c",
        "src/video/x11/SDL_x11xinput2.c",
        "src/video/x11/edid-parse.c",
        "src/video/x11/imKStoUCS.c",
    ],
    hdrs = [
        "include/SDL.h",
        "include/SDL_assert.h",
        "include/SDL_atomic.h",
        "include/SDL_audio.h",
        "include/SDL_bits.h",
        "include/SDL_blendmode.h",
        "include/SDL_clipboard.h",
        "include/SDL_config.h",
        "include/SDL_cpuinfo.h",
        "include/SDL_endian.h",
        "include/SDL_error.h",
        "include/SDL_events.h",
        "include/SDL_filesystem.h",
        "include/SDL_gamecontroller.h",
        "include/SDL_gesture.h",
        "include/SDL_haptic.h",
        "include/SDL_hints.h",
        "include/SDL_joystick.h",
        "include/SDL_keyboard.h",
        "include/SDL_keycode.h",
        "include/SDL_loadso.h",
        "include/SDL_log.h",
        "include/SDL_main.h",
        "include/SDL_messagebox.h",
        "include/SDL_mouse.h",
        "include/SDL_mutex.h",
        "include/SDL_name.h",
        "include/SDL_opengl.h",
        "include/SDL_opengl_glext.h",
        "include/SDL_pixels.h",
        "include/SDL_platform.h",
        "include/SDL_power.h",
        "include/SDL_quit.h",
        "include/SDL_rect.h",
        "include/SDL_render.h",
        "include/SDL_revision.h",
        "include/SDL_rwops.h",
        "include/SDL_scancode.h",
        "include/SDL_shape.h",
        "include/SDL_stdinc.h",
        "include/SDL_surface.h",
        "include/SDL_system.h",
        "include/SDL_syswm.h",
        "include/SDL_thread.h",
        "include/SDL_timer.h",
        "include/SDL_touch.h",
        "include/SDL_version.h",
        "include/SDL_video.h",
        "include/begin_code.h",
        "include/close_code.h",
        "src/SDL_assert_c.h",
        "src/SDL_error_c.h",
        "src/SDL_internal.h",
        "src/audio/SDL_audio_c.h",
        "src/audio/SDL_sysaudio.h",
        "src/audio/SDL_wave.h",
        "src/core/linux/SDL_dbus.h",
        "src/core/linux/SDL_evdev.h",
        "src/core/linux/SDL_fcitx.h",
        "src/core/linux/SDL_ibus.h",
        "src/core/linux/SDL_ime.h",
        "src/core/linux/SDL_udev.h",
        "src/dynapi/SDL_dynapi.h",
        "src/events/SDL_clipboardevents_c.h",
        "src/events/SDL_dropevents_c.h",
        "src/events/SDL_events_c.h",
        "src/events/SDL_gesture_c.h",
        "src/events/SDL_keyboard_c.h",
        "src/events/SDL_mouse_c.h",
        "src/events/SDL_touch_c.h",
        "src/events/SDL_windowevents_c.h",
        "src/events/default_cursor.h",
        "src/events/scancodes_darwin.h",
        "src/events/scancodes_linux.h",
        "src/events/scancodes_xfree86.h",
        "src/haptic/SDL_haptic_c.h",
        "src/haptic/SDL_syshaptic.h",
        "src/joystick/SDL_gamecontrollerdb.h",
        "src/joystick/SDL_joystick_c.h",
        "src/joystick/SDL_sysjoystick.h",
        "src/joystick/linux/SDL_sysjoystick_c.h",
        "src/libm/math_libm.h",
        "src/libm/math_private.h",
        "src/render/SDL_sysrender.h",
        "src/render/SDL_yuv_sw_c.h",
        "src/render/opengl/SDL_glfuncs.h",
        "src/render/opengl/SDL_shaders_gl.h",
        "src/render/software/SDL_blendfillrect.h",
        "src/render/software/SDL_blendline.h",
        "src/render/software/SDL_blendpoint.h",
        "src/render/software/SDL_draw.h",
        "src/render/software/SDL_drawline.h",
        "src/render/software/SDL_drawpoint.h",
        "src/render/software/SDL_render_sw_c.h",
        "src/render/software/SDL_rotate.h",
        "src/thread/SDL_systhread.h",
        "src/thread/SDL_thread_c.h",
        "src/thread/pthread/SDL_sysmutex_c.h",
        "src/thread/pthread/SDL_systhread_c.h",
        "src/timer/SDL_timer_c.h",
        "src/video/SDL_RLEaccel_c.h",
        "src/video/SDL_blit.h",
        "src/video/SDL_blit_auto.h",
        "src/video/SDL_blit_copy.h",
        "src/video/SDL_blit_slow.h",
        "src/video/SDL_pixels_c.h",
        "src/video/SDL_rect_c.h",
        "src/video/SDL_shape_internals.h",
        "src/video/SDL_sysvideo.h",
        "src/video/x11/SDL_x11clipboard.h",
        "src/video/x11/SDL_x11dyn.h",
        "src/video/x11/SDL_x11events.h",
        "src/video/x11/SDL_x11framebuffer.h",
        "src/video/x11/SDL_x11keyboard.h",
        "src/video/x11/SDL_x11messagebox.h",
        "src/video/x11/SDL_x11modes.h",
        "src/video/x11/SDL_x11mouse.h",
        "src/video/x11/SDL_x11opengl.h",
        "src/video/x11/SDL_x11opengles.h",
        "src/video/x11/SDL_x11shape.h",
        "src/video/x11/SDL_x11sym.h",
        "src/video/x11/SDL_x11touch.h",
        "src/video/x11/SDL_x11video.h",
        "src/video/x11/SDL_x11window.h",
        "src/video/x11/SDL_x11xinput2.h",
        "src/video/x11/edid.h",
        "src/video/x11/imKStoUCS.h",
    ],
    copts = [
        "-DUSING_GENERATED_CONFIG_H",
        "-DHAVE_LINUX_VERSION_H",
        "-D_REENTRANT",
    ],
    includes = ["include"],
    linkopts = [
        "-ldl",
        "-lm",
        "-lpthread",
        "-lX11",
        "-lXext",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "SDL_test",
    srcs = [
        "src/test/SDL_test_assert.c",
        "src/test/SDL_test_common.c",
        "src/test/SDL_test_compare.c",
        "src/test/SDL_test_crc32.c",
        "src/test/SDL_test_font.c",
        "src/test/SDL_test_fuzzer.c",
        "src/test/SDL_test_harness.c",
        "src/test/SDL_test_imageBlit.c",
        "src/test/SDL_test_imageBlitBlend.c",
        "src/test/SDL_test_imageFace.c",
        "src/test/SDL_test_imagePrimitives.c",
        "src/test/SDL_test_imagePrimitivesBlend.c",
        "src/test/SDL_test_log.c",
        "src/test/SDL_test_md5.c",
        "src/test/SDL_test_random.c",
    ],
    deps = [":SDL"],
)

cc_binary(
    name = "checkkeys",
    srcs = ["test/checkkeys.c"],
    visibility = ["//visibility:public"],
    deps = [":SDL"],
)

#cc_test(
#name = "testatomic",
#srcs = ["test/testatomic.c"],
#deps = [":SDL"],
#visibility = ["//visibility:public"],
#)

cc_test(
    name = "testver",
    srcs = ["test/testver.c"],
    deps = [":SDL"],
)

cc_test(
    name = "testthread",
    srcs = ["test/testthread.c"],
    deps = [":SDL"],
)

cc_test(
    name = "testtimer",
    srcs = ["test/testtimer.c"],
    deps = [":SDL"],
)

#cc_test(
#name = "testsem",
#srcs = ["test/testsem.c"],
#deps = [":SDL"],
#)

cc_test(
    name = "testpower",
    srcs = ["test/testpower.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_test(
    name = "testqsort",
    srcs = ["test/testqsort.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_test(
    name = "torturethread",
    srcs = ["test/torturethread.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testkeys",
    srcs = ["test/testkeys.c"],
    deps = [":SDL"],
)

#Missing environment variables?
#cc_test(
#name = "testfilesystem",
#srcs = ["test/testfilesystem.c"],
#deps = [":SDL"],
#)

cc_test(
    name = "testfile",
    srcs = ["test/testfile.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testwm2",
    srcs = ["test/testwm2.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testviewport",
    srcs = ["test/testviewport.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "teststreaming",
    srcs = ["test/teststreaming.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testspriteminimal",
    srcs = ["test/testspriteminimal.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testsprite2",
    srcs = ["test/testsprite2.c"],
    deps = [
        "SDL_test",
        ":SDL",
    ],
)

cc_binary(
    name = "testshape",
    srcs = ["test/testshape.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testshader",
    srcs = ["test/testshader.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testscale",
    srcs = ["test/testscale.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testrendertarget",
    srcs = ["test/testrendertarget.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testrendercopyex",
    srcs = ["test/testrendercopyex.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testrelative",
    srcs = ["test/testrelative.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_test(
    name = "testplatform",
    srcs = ["test/testplatform.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testoverlay2",
    srcs = ["test/testoverlay2.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testnative",
    srcs = [
        "test/testnative.c",
        "test/testnative.h",
        "test/testnativex11.c",
    ],
    deps = [":SDL"],
)

cc_binary(
    name = "testmessage",
    srcs = ["test/testmessage.c"],
    deps = [":SDL"],
)

# Test doesn't seem to end?
#cc_test(
#name = "testlock",
#srcs = ["test/testlock.c"],
#deps = [":SDL"],
#)

cc_binary(
    name = "testime",
    srcs = ["test/testime.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testjoystick",
    srcs = ["test/testjoystick.c"],
    deps = [":SDL"],
)

# Directory/path issue
#cc_test(
#name = "testiconv",
#srcs = ["test/testiconv.c"],
#deps = [":SDL"],
#data = [
#"test/utf8.txt"
#],
#)

cc_binary(
    name = "testgles",
    srcs = ["test/testgles.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testgl2",
    srcs = ["test/testgl2.c"],
    copts = ["-DHAVE_OPENGL"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testgles2",
    srcs = ["test/testgles2.c"],
    copts = ["-DHAVE_OPENGL"],
    deps = [":SDL"],
)

cc_binary(
    name = "testhittesting",
    srcs = ["test/testhittesting.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testbounds",
    srcs = ["test/testbounds.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testcustomcursor",
    srcs = ["test/testcustomcursor.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testdisplayinfo",
    srcs = ["test/testdisplayinfo.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testdraw2",
    srcs = ["test/testdraw2.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_binary(
    name = "testdrawchessboard",
    srcs = ["test/testdrawchessboard.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testdropfile",
    srcs = ["test/testdropfile.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

cc_test(
    name = "testerror",
    srcs = ["test/testerror.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testgamecontroller",
    srcs = ["test/testgamecontroller.c"],
    deps = [":SDL"],
)

cc_binary(
    name = "testintersections",
    srcs = ["test/testintersections.c"],
    deps = [
        ":SDL",
        ":SDL_test",
    ],
)

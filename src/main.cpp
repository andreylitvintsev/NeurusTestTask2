// Вообще предпочитаю писать ООП, но для маленькой программы решил что будет избыточно

#include <gst/gst.h>
#include <glib.h>
#include <sstream>
#include <iostream>

// fields
static GstElement* _mixer{};
static GstPad* _sinkPad0{};
static GstPad* _sinkPad1{};

static gint64 _toggleStep{};
static gint64 _nextToggleTime{};

// functions
static gboolean BusCallback(GstBus* bus, GstMessage* msg, gpointer data);
static gboolean TryToggleCallback(GstElement *pipeline);


int main(int argc, char* argv[]) {
    // Args parsing
    if (argc != 4) {
        g_print("Run me with --help to see the Application options appended.\n");
        return 1;
    }

    gchar* videoUri1{};
    gchar* videoUri2{};

    GOptionContext* ctx{};
    GError* err{};
    GOptionEntry entries[] = {
        {"uri1", 0, 0, G_OPTION_ARG_STRING, &videoUri1, "Video uri1", "URI"},
        {"uri2", 0, 0, G_OPTION_ARG_STRING, &videoUri2, "Video uri2", "URI"},
        {"step", 0, 0, G_OPTION_ARG_INT64, &_toggleStep, "Toggle step", "MS"},
        {nullptr}
    };

    ctx = g_option_context_new ("- Test task 2");
    g_option_context_add_main_entries (ctx, entries, nullptr);
    g_option_context_add_group (ctx, gst_init_get_option_group());
    if (!g_option_context_parse (ctx, &argc, &argv, &err)) {
        g_print("Failed to initialize: %s\n", err->message);
        g_clear_error(&err);
        g_option_context_free(ctx);
        return 1;
    }
    g_option_context_free(ctx);
    _nextToggleTime = _toggleStep;

    // Initialization
    GMainLoop* loop{};

    GstElement* pipeline{};
    GstBus* bus{};
    guint bus_watch_id{};

    gst_init(&argc, &argv);

    loop = g_main_loop_new(nullptr, FALSE);

    /* Create gstreamer elements */
    std::stringstream gstLaunchString{};
    gstLaunchString
        << "videomixer name=mixer ! videoconvert ! timeoverlay halignment=center valignment=top ! fpsdisplaysink "
        << "uridecodebin uri=" << videoUri1 << " ! videoscale ! video/x-raw,width=320,height=240 ! mixer. "
        << "uridecodebin uri=" << videoUri2 << " ! videoscale ! video/x-raw,width=320,height=240 ! mixer. ";
    std::cout << _toggleStep << std::endl;
    pipeline = gst_parse_launch(gstLaunchString.str().c_str(), nullptr);

    _mixer = gst_bin_get_by_name(GST_BIN_CAST(pipeline), "mixer");
    _sinkPad0 = gst_element_get_static_pad(_mixer, "sink_0");
    _sinkPad1 = gst_element_get_static_pad(_mixer, "sink_1");

    if (!pipeline || !_mixer || !_sinkPad0 || !_sinkPad1) {
        g_printerr("One element could not be initialized. Exiting.\n");
        return -1;
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));
    bus_watch_id = gst_bus_add_watch(bus, BusCallback, loop);
    gst_object_unref(bus);

    // Set the pipeline to "playing" state
    g_print("Now playing: %s\n", argv[1]);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Start main loop
    g_print("Running...\n");
    guint timeoutId = g_timeout_add(100, (GSourceFunc) TryToggleCallback, pipeline);
    g_main_loop_run(loop);

    // Clean up
    g_source_remove(timeoutId);

    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);

    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT (pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);

    return 0;
}

static gboolean BusCallback(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop* loop = static_cast<GMainLoop*>(data);

    switch (GST_MESSAGE_TYPE (msg)) {

        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            g_main_loop_quit(loop);
            break;

        case GST_MESSAGE_ERROR: {
            gchar* debug{};
            GError* error{};

            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);

            g_printerr("Error: %s\n", error->message);
            g_error_free(error);

            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }

    return true;
}

static gboolean TryToggleCallback(GstElement *pipeline) {
    gint64 videoPlayProgress{};
    if (!gst_element_query_position(pipeline, GST_FORMAT_TIME, &videoPlayProgress))
        return true;

    if (videoPlayProgress / GST_MSECOND < _nextToggleTime)
        return true;

    _nextToggleTime += _toggleStep;

    gint zOrderValue{};
    g_object_get(_sinkPad0, "zorder", &zOrderValue, nullptr);
    zOrderValue = !zOrderValue;

    g_object_set(_sinkPad0, "zorder", zOrderValue, nullptr);
    g_object_set(_sinkPad1, "zorder", !zOrderValue, nullptr);

    return true;
}
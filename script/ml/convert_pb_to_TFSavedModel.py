import tensorflow as tf
from tensorflow.python.saved_model import signature_constants
from tensorflow.python.saved_model import tag_constants

export_dir = './saved'
graph_pb = 'frozen_inference_graph.pb'

builder = tf.saved_model.builder.SavedModelBuilder(export_dir)

with tf.gfile.GFile(graph_pb, "rb") as f:
    graph_def = tf.GraphDef()
    graph_def.ParseFromString(f.read())

sigs = {}

with tf.Session(graph=tf.Graph()) as sess:
    # name="" is important to ensure we don't get spurious prefixing
    tf.import_graph_def(graph_def, name="")
    g = tf.get_default_graph()
    image_tensor = g.get_tensor_by_name("image_tensor:0")
    detection_boxes = g.get_tensor_by_name("detection_boxes:0")
    detection_scores = g.get_tensor_by_name("detection_scores:0")
    detection_classes = g.get_tensor_by_name("detection_classes:0")
    num_detections = g.get_tensor_by_name("num_detections:0")

    sigs[signature_constants.DEFAULT_SERVING_SIGNATURE_DEF_KEY] = \
        tf.saved_model.signature_def_utils.predict_signature_def(
            {"image_tensor": image_tensor}, {"detection_boxes": detection_boxes, "detection_scores": detection_scores, "detection_classes": detection_classes, "num_detections": num_detections})

    builder.add_meta_graph_and_variables(sess,
                                         [tag_constants.SERVING],
                                         signature_def_map=sigs)

builder.save()


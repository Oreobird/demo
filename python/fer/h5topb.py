# -*- coding: utf-8 -*-

import tensorflow as tf
import os
import os.path as osp

def freeze_session(session, keep_var_names=None, output_names=None, clear_devices=True):
    from tensorflow.python.framework.graph_util import convert_variables_to_constants

    graph = session.graph
    with graph.as_default():
        freeze_var_names = list(set(v.op.name for v in tf.global_variables()).difference(keep_var_names or []))

    output_names = output_names or []
    output_names += [v.op.name for v in tf.global_variables()]
    input_graph_def = graph.as_graph_def()

    if clear_devices:
        for node in input_graph_def.node:
            node.device = ""

    frozen_graph = convert_variables_to_constants(session, input_graph_def,output_names, freeze_var_names)
    return frozen_graph


#output_graph_name = 'fer2013_model.pb'
#output_graph_name = 'mafa_12_model.pb'
output_graph_name = 'cofw_model_5_class.pb'
#output_fld = "E:/ml/fod/model/"
output_fld = "E:/ml/fod/model/"
#weight_file_path = "E:/ml/fer/model/fer2013_model.h5"
#weight_file_path = "E:/ml/fer/model/mafa_12_model.h5"
weight_file_path = "E:/ml/fod/model/fod_cofw_model_5_class.h5"

if not os.path.isdir(output_fld):
    os.mkdir(output_fld)

#K.set_learning_phase(0)
tf.keras.backend.set_learning_phase(1)
#net_model = load_model(weight_file_path)
net_model = tf.keras.models.load_model(weight_file_path)

print('input is :', net_model.input.name)
print('output is:', net_model.output.name)

sess = tf.keras.backend.get_session()
frozen_graph = freeze_session(sess, output_names=[net_model.output.op.name])
from tensorflow.python.framework import graph_io
graph_io.write_graph(frozen_graph, output_fld, output_graph_name, as_text=False)

#tf.train.write_graph(frozen_graph, output_fld, "fer_model.pbtxt")

print('saved the constant graph (ready for inference) at: ', osp.join(output_fld, output_graph_name))
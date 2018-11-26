# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import os
import sys

from google.protobuf import text_format

from tensorflow.core.framework import graph_pb2
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import graph_io
from tensorflow.python.platform import app
from tensorflow.python.platform import gfile
from tensorflow.python.tools import optimize_for_inference_lib
import tensorflow as tf
import os.path as osp

def split_dir_and_name(file_path):
    tokens = file_path.split('/')
    dir = ""
    dir_tokens = tokens[:-1]
    name = tokens[-1]
    for token in dir_tokens:
        dir += token + '/'
    return dir, name

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

    
def h5_to_pb(h5_path, pb_path):
    output_fld = osp.dirname(pb_path)
    pb_name = osp.basename(pb_path)
    
    if not os.path.isdir(output_fld):
        os.mkdir(output_fld)

    tf.keras.backend.set_learning_phase(1)
    net_model = tf.keras.models.load_model(h5_path)
    
    print('input is :', net_model.input.name)
    print('output is:', net_model.output.name)
    
    sess = tf.keras.backend.get_session()
    frozen_graph = freeze_session(sess, output_names=[net_model.output.op.name])
    from tensorflow.python.framework import graph_io
    graph_io.write_graph(frozen_graph, output_fld, pb_name, as_text=False)
    
    print('saved the constant graph (ready for inference) at: ', osp.join(output_fld, pb_name))


def gen_pb_txt(pb_path, pbtxt_path):
    output_fld = osp.dirname(pbtxt_path)
    output_graph_name = osp.basename(pbtxt_path)

    # Read the graph.
    with tf.gfile.FastGFile(pb_path, 'rb') as f:
        graph_def = tf.GraphDef()
        graph_def.ParseFromString(f.read())
    
    # Save dropout input to list
    drop_out_input = []
    dp_idx = 0
    for i in reversed(range(len(graph_def.node))):
        if 'dropout' in graph_def.node[i].name:
            idx = 0
            for item in graph_def.node[i].input:
                if not 'dropout' in item:
                    drop_out_input.append(graph_def.node[i].input[idx])
                idx += 1
    
    # Remove Const nodes.
    for i in reversed(range(len(graph_def.node))):
        # Del Const op
        if graph_def.node[i].op == 'Const':
            del graph_def.node[i]
        
        # Modify Flatten
        if graph_def.node[i].op == 'StridedSlice' or graph_def.node[i].op == 'Pack':
            del graph_def.node[i]
        if graph_def.node[i].op == 'Reshape':
            graph_def.node[i].op = 'Flatten'
            input = graph_def.node[i].input
            idx = 0
            for item in input:
                if 'flatten/Reshape' in item:
                    del graph_def.node[i].input[idx]
                idx += 1
        
        # Del dropout
        if 'dropout' in graph_def.node[i].name:
            del graph_def.node[i]
        
        # Modify input has dropout
        idx = 0
        for item in graph_def.node[i].input:
            if 'dropout' in item:
                graph_def.node[i].input[idx] = drop_out_input[dp_idx]
                dp_idx += 1
            idx += 1
        
        # Del attr
        for attr in ['T', 'data_format', 'Tshape', 'N', 'Tidx', 'Tdim',
                     'use_cudnn_on_gpu', 'Index', 'Tperm', 'is_training',
                     'Tpaddings']:
            if attr in graph_def.node[i].attr:
                del graph_def.node[i].attr[attr]
    
    # Save as text.
    tf.train.write_graph(graph_def, output_fld, output_graph_name, as_text=True)
    




FLAGS = None

def optimize_pb():
    if not gfile.Exists(FLAGS.raw_pb):
        print("Input graph file '" + FLAGS.raw_pb + "' does not exist!")
        return -1

    input_graph_def = graph_pb2.GraphDef()
    with gfile.Open(FLAGS.raw_pb, "rb") as f:
        data = f.read()
        if FLAGS.frozen_graph:
            input_graph_def.ParseFromString(data)
        else:
            text_format.Merge(data.decode("utf-8"), input_graph_def)

    output_graph_def = optimize_for_inference_lib.optimize_for_inference(
                          input_graph_def,
                          FLAGS.input_names.split(","),
                          FLAGS.output_names.split(","),
                          FLAGS.placeholder_type_enum,
                          FLAGS.toco_compatible)

    if FLAGS.frozen_graph:
        f = gfile.FastGFile(FLAGS.opt_pb, "w")
        f.write(output_graph_def.SerializeToString())
    else:
        graph_io.write_graph(output_graph_def,
                             os.path.dirname(FLAGS.opt_pb),
                             os.path.basename(FLAGS.opt_pb))
    print('saved the opt pb file at: ', FLAGS.opt_pb)

def main(unused_args):
    h5_to_pb(FLAGS.h5, FLAGS.raw_pb)
    optimize_pb()
    gen_pb_txt(FLAGS.opt_pb, FLAGS.pb_txt)
    return 0

def parse_args():
    """Parses command line arguments."""
    parser = argparse.ArgumentParser()
    parser.register("type", "bool", lambda v: v.lower() == "true")
    parser.add_argument(
        "--h5",
        type=str,
        default="",
        help="Keras h5 file to convert."
    )
    parser.add_argument(
        "--raw_pb",
        type=str,
        default="",
        help="pb file converted from h5."
    )
    parser.add_argument(
        "--opt_pb",
        type=str,
        default="",
        help="pb optimized for inference."
    )
    parser.add_argument(
        "--pb_txt",
        type=str,
        default="",
        help="pbtxt file."
    )
    parser.add_argument(
        "--input_names",
        type=str,
        default="",
        help="Input node names, comma separated.")
    parser.add_argument(
        "--output_names",
        type=str,
        default="",
        help="Output node names, comma separated.")
    parser.add_argument(
        "--frozen_graph",
        nargs="?",
        const=True,
        type="bool",
        default=True,
        help="""\
        If true, the input graph is a binary frozen GraphDef
        file; if false, it is a text GraphDef proto file.\
        """)
    parser.add_argument(
        "--placeholder_type_enum",
        type=int,
        default=dtypes.float32.as_datatype_enum,
        help="The AttrValue enum to use for placeholders.")
    parser.add_argument(
        "--toco_compatible",
        type=bool,
        default=False,
        help="""\
        If true, only use ops compatible with Tensorflow
        Lite Optimizing Converter.\
        """)
    return parser.parse_known_args()


if __name__ == "__main__":
    FLAGS, unparsed = parse_args()
    app.run(main=main, argv=[sys.argv[0]] + unparsed)
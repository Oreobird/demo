import tensorflow as tf

#output_graph_name = 'fer2013_model_opt.pbtxt'
output_graph_name = 'cofw_model_5_class_opt.pbtxt'
output_fld = "E:/ml/fod/model/"
#weight_file_path = "E:/ml/fer/model/fer2013_model_opt.pb"
weight_file_path = "E:/ml/fod/model/cofw_model.pb"

def gen_pb_txt(pb_path, pbtxt_path):
    output_fld, output_graph_name = split_dir_and_name(pbtxt_path)
    
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
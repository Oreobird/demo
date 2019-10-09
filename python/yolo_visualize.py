import matplotlib.pyplot as plt
import argparse


class LogParser:
    
    def __init__(self, log_file):
        self._log_file = log_file
        self.__tokens = ['loss', 'avg', 'rate', 'seconds', 'images']
        
    def __parse(self, idx, key='images'):
        values = []
        with open(self._log_file, 'r') as log_fp:
            for line in log_fp:
                if 'Syncing' in line:
                    continue
                if 'nan' in line:
                    continue
                if key in line:
                    value = line.split(' ')[idx].rstrip(',')
                    values.append(float(value))
        return values
        
    def avg_loss(self):
        return self.__parse(2, 'avg')
    
    def rate(self):
        return self.__parse(4, 'rate')
    
    def iou(self):
        return self.__parse(4, 'IOU')

    def class_val(self):
        return self.__parse(6, 'IOU')
     
    def obj(self):
        return self.__parse(8, 'IOU')
       
    def no_obj(self):
        return self.__parse(11, 'IOU')
    

class LogVisualizer:
    
    def __init__(self, log_parser, save_path=None, show=False):
        self.__log_parser = log_parser
        self.__save_path = save_path
        self.__show = show
		
    def __visualize(self, fn, label='', save_path=None):
        fig = plt.figure()
        ax = fig.add_subplot(1, 1, 1)
        y = fn()
        x_num = len(y)
        x = [i for i in range(x_num)]
        ax.plot(x, y, label=label)
        ax.legend(loc = 'best')
        ax.set_title('The {} curves'.format(label))
        ax.set_xlabel('batches')
        
        if save_path is not None:
            fig.savefig(save_path + '/' + label)
        
        if self.__show:
            plt.show()

    def avg_loss(self):
        self.__visualize(self.__log_parser.avg_loss, 'avg_loss')

    def rate(self):
        self.__visualize(self.__log_parser.rate, 'rate')

    def iou(self):
        self.__visualize(self.__log_parser.iou, 'IOU')

    def class_val(self):
        self.__visualize(self.__log_parser.class_val, 'Class')

    def obj(self):
        self.__visualize(self.__log_parser.obj, 'Obj')

    def no_obj(self):
        self.__visualize(self.__log_parser.no_obj, 'No Obj')


def parse_args():
    """Parses command line arguments."""
    parser = argparse.ArgumentParser()
    parser.register("type", "bool", lambda v: v.lower() == "true")
    parser.add_argument(
        "--log_file",
        type=str,
        default="",
        help="yolo log file to parse."
    )
    parser.add_argument(
        "--save_path",
        type=str,
        default="",
        help="yolo visualized curve image save path."
    )
    parser.add_argument(
        "--show",
        type=bool,
        default=False,
        help="yolo show curve image or not."
    )
    return parser.parse_known_args()


def run(flags):
    parser = LogParser(flags.log_file)
    visualizer = LogVisualizer(parser, flags.save_path, flags.show)
    visualizer.avg_loss()
    visualizer.iou()
    
    
if __name__ == '__main__':
    FLAGS, _ = parse_args()
    run(FLAGS)

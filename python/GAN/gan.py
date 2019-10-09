from __future__ import print_function, division

import tensorflow as tf
import matplotlib.pyplot as plt
import sys
import numpy as np
import cv2
import os
import random


class GAN():
    def __init__(self):
        self.img_rows = 48
        self.img_cols = 48
        self.channels = 1
        self.img_shape = (self.img_rows, self.img_cols, self.channels)
        self.latent_dim = 100
        
        optimizer = tf.keras.optimizers.Adam(0.0002, 0.5)
        
        self.discriminator = self.build_discriminator()
        self.discriminator.compile(optimizer=optimizer,
                                   loss='binary_crossentropy',
                                   metrics=['accuracy'])
        
        self.generator = self.build_generator()
        
        # generator takes noise as input and generates imgs
        z = tf.keras.layers.Input(shape=(self.latent_dim,))
        img = self.generator(z)
        
        # for the combined model only train the generator
        self.discriminator.trainable = False
        
        # discriminator takes generated images as input and determines validity
        validity = self.discriminator(img)
        
        # Train generator to fool discriminator
        self.combined = tf.keras.Model(z, validity)
        self.combined.compile(optimizer=optimizer,
                              loss='binary_crossentropy')
    
    def build_generator(self):
        model = tf.keras.Sequential([
            tf.keras.layers.Dense(units=256, input_dim=self.latent_dim),
            tf.keras.layers.LeakyReLU(alpha=0.2),
            tf.keras.layers.BatchNormalization(momentum=0.8),
            tf.keras.layers.Dense(512),
            tf.keras.layers.LeakyReLU(alpha=0.2),
            tf.keras.layers.BatchNormalization(momentum=0.8),
            tf.keras.layers.Dense(1024),
            tf.keras.layers.LeakyReLU(alpha=0.2),
            tf.keras.layers.BatchNormalization(momentum=0.8),
            tf.keras.layers.Dense(np.prod(self.img_shape), activation='tanh'),
            tf.keras.layers.Reshape(self.img_shape)
        ])
        
        model.summary()
        noise = tf.keras.layers.Input(shape=(self.latent_dim,))
        img = model(noise)
        
        return tf.keras.Model(noise, img)
    
    
    def build_discriminator(self):
        model = tf.keras.Sequential([
            tf.keras.layers.Flatten(input_shape=self.img_shape),
            tf.keras.layers.Dense(512),
            tf.keras.layers.LeakyReLU(alpha=0.2),
            tf.keras.layers.Dense(256),
            tf.keras.layers.LeakyReLU(alpha=0.2),
            tf.keras.layers.Dense(1, activation='sigmoid')
        ])
        
        model.summary()
        
        img = tf.keras.layers.Input(shape=self.img_shape)
        validity = model(img)
        
        return tf.keras.Model(img, validity)

    def load_imgpath_labels(self, filename, labels_num=1, shuffle=True):
        imgpath = []
        labels = []
    
        with open(os.path.join(filename)) as f:
            lines_list = f.readlines()
            if shuffle:
                random.shuffle(lines_list)
        
            for lines in lines_list:
                line = lines.rstrip().split(',')
            
                label = None
                if labels_num == 1:
                    label = int(line[1])
                else:
                    for i in range(labels_num):
                        label.append(int(line[i + 1]))
                imgpath.append(line[0])
                labels.append(label)
    
        return np.array(imgpath), np.array(labels)


    def get_input_img(self, filename):
        img = cv2.imread(filename, cv2.IMREAD_GRAYSCALE)
        img = cv2.resize(img, (48, 48))
        return img
    
    def get_data(self, data_file, shuffle=True):
        filenames, labels = self.load_imgpath_labels(data_file, shuffle)
        x_train = []
        for i in range(0, len(filenames)):
            img = self.get_input_img(filenames[i])
            # img = np.expand_dims(img, 2)
            x_train.append(img)
        x_train = np.array(x_train, dtype=np.float32)
            
        return x_train
    
    def train(self, epochs, batch_size=128, sample_interval=50):
        # mnist = tf.keras.datasets.mnist
        #
        # (x_train, _), (_, _) = mnist.load_data()
        
        x_train = self.get_data("E:/ml/fer/merge_train.txt")
        
        x_train = x_train / 127.5 - 1.
        x_train = np.expand_dims(x_train, axis=3)
        
        # adversarial ground truths
        valid = np.ones((batch_size, 1))
        fake = np.zeros((batch_size, 1))
        
        for epoch in range(epochs):
            # Train Discriminator
            idx = np.random.randint(0, x_train.shape[0], batch_size)
            imgs = x_train[idx]
            noise = np.random.normal(0, 1, (batch_size, self.latent_dim))
            gen_imgs = self.generator.predict(noise)
            
            d_loss_real = self.discriminator.train_on_batch(imgs, valid)
            d_loss_fake = self.discriminator.train_on_batch(gen_imgs, fake)
            d_loss = 0.5 * np.add(d_loss_real, d_loss_fake)

            # Train Generator
            noise = np.random.normal(0, 1, (batch_size, self.latent_dim))
            g_loss = self.combined.train_on_batch(noise, valid)
            
            # plot the progress
            print("%d [D loss: %f, acc: %.2f%%] [G loss: %f]" % (epoch, d_loss[0], 100*d_loss[1], g_loss))
            
            if epoch % sample_interval == 0:
                self.sample_images(epoch)
                
    def sample_images(self, epoch):
        r, c = 5, 5
        noise = np.random.normal(0, 1, (r * c, self.latent_dim))
        gen_imgs = self.generator.predict(noise)
        
        gen_imgs = 0.5 * gen_imgs + 0.5
        
        fig, axs = plt.subplots(r, c)
        cnt = 0
        for i in range(r):
            for j in range(c):
                axs[i, j].imshow(gen_imgs[cnt, :,:,0], cmap='gray')
                axs[i,j].axis('off')
                cnt += 1
        fig.savefig("E:/demo/python/GAN/images/%d.jpg" % epoch)
        plt.close()

    
if __name__ == '__main__':
    gan = GAN()
    gan.train(epochs=20000, batch_size=32, sample_interval=200)
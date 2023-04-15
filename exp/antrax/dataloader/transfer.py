import cv2
import h5py
import numpy as np

cap = cv2.VideoCapture('sample.mp4')

width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
num_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT)) // 10
with h5py.File('sample.hdf5', 'w') as f:
    dset = f.create_dataset('test', shape=(num_frames, width, height, 3), dtype=np.uint8, chunks=True)
    for i in range(num_frames):
        ret, frame = cap.read()
        if ret:
            dset[i] = frame
cap.release()

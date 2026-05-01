import os
import shutil
import tensorflow as tf
from tensorflow.keras import layers, models

base_dir = r'd:\project\Rover_Project_BackEnd'
dataset_dir = os.path.join(base_dir, 'dataset')

classes = {
    'Healthy_Chilli': os.path.join(base_dir, 'Chilli sample pic', 'Healthy_sample_chilli'),
    'Disease_Chilli': os.path.join(base_dir, 'Chilli sample pic', 'Disease_sample_chilli'),
    'Healthy_Tomato': os.path.join(base_dir, 'tomato sample pic', 'Healthy_Tomato'),
    'Disease_Tomato': os.path.join(base_dir, 'tomato sample pic', 'disease_tomato')
}

print("Consolidating dataset...")
if not os.path.exists(dataset_dir):
    os.makedirs(dataset_dir)
    for class_name, source_path in classes.items():
        dest_path = os.path.join(dataset_dir, class_name)
        os.makedirs(dest_path, exist_ok=True)
        if os.path.exists(source_path):
            for file in os.listdir(source_path):
                if file.endswith(('.png', '.jpg', '.jpeg', '.JPG', '.PNG')):
                    shutil.copy(os.path.join(source_path, file), os.path.join(dest_path, file))

print("Loading dataset into TensorFlow...")
batch_size = 2 # Very small batch size since we have few images
img_height = 224
img_width = 224

train_ds = tf.keras.utils.image_dataset_from_directory(
  dataset_dir,
  seed=123,
  image_size=(img_height, img_width),
  batch_size=batch_size)

class_names = train_ds.class_names
print("Classes found:", class_names)

# Data Augmentation layer
data_augmentation = tf.keras.Sequential([
  layers.RandomFlip("horizontal_and_vertical"),
  layers.RandomRotation(0.2),
  layers.RandomZoom(0.2),
])

print("Building model...")
# Build a simple CNN
model = models.Sequential([
  tf.keras.Input(shape=(img_height, img_width, 3)),
  data_augmentation,
  layers.Rescaling(1./255),
  layers.Conv2D(16, 3, padding='same', activation='relu'),
  layers.MaxPooling2D(),
  layers.Conv2D(32, 3, padding='same', activation='relu'),
  layers.MaxPooling2D(),
  layers.Flatten(),
  layers.Dense(64, activation='relu'),
  layers.Dense(len(class_names))
])

model.compile(optimizer='adam',
              loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
              metrics=['accuracy'])

print("Training model...")
epochs = 15
history = model.fit(
  train_ds,
  epochs=epochs
)

model_path = os.path.join(base_dir, 'plant_model.h5')
model.save(model_path)
print(f"Model successfully saved to {model_path}!")

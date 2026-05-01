# AI Backend & Training Guide 🧠

The rover's brain is a Convolutional Neural Network (CNN) built with **TensorFlow and Keras**.

## 1. How the Analysis Works
1. The **ESP32-CAM** captures a 640x480 JPEG.
2. It sends the image via an HTTP POST request to the laptop's `/upload` endpoint.
3. The server resizes the image to **224x224** (the input size the AI expects).
4. The AI predicts the class and sends the result to a shared variable.
5. The **Main ESP32** fetches this result via a GET request to `/get_result`.

## 2. Managing the Dataset
The AI is currently trained on four folders in your `Rover_Project_BackEnd`:
- `Healthy_Chilli`
- `Disease_Chilli`
- `Healthy_Tomato`
- `Disease_Tomato`

Whenever the rover captures a new image, it is automatically saved to the **`captured_images/`** folder. 
- **Pro Tip**: After a day of testing, take the best images from `captured_images/`, move them into the correct plant folders, and retrain the model to make it smarter!

## 3. Retraining the Model
If you add more images or new plant types:
1. Open your terminal in the backend folder.
2. Run: `python train_model.py`
3. The script will:
   - Consolidate all images into a unified `dataset/` folder.
   - Apply **Data Augmentation** (rotation, flipping, zooming).
   - Train for 15 epochs.
   - Save a new `plant_model.h5`.
4. Restart the server (`python server.py`) to load the new brain.

## 4. Class Names
The server uses alphabetical ordering for class names. Current order:
1. `Disease_Chilli`
2. `Disease_Tomato`
3. `Healthy_Chilli`
4. `Healthy_Tomato`

If you add a new folder (e.g., `Disease_Potato`), ensure you update the `class_names` list in `server.py` to match the alphabetical order of your folders.

from flask import Flask, request, jsonify
import tensorflow as tf
import numpy as np
import cv2
import os
import time
import random

app = Flask(__name__)

# Load your trained model
try:
    model = tf.keras.models.load_model('plant_model.h5')
    print("Model loaded successfully.")
except Exception as e:
    print("Warning: plant_model.h5 not found! Running in Mock Mode.")
    model = None

# Automatically detect classes from the dataset folder
dataset_path = 'dataset'
if os.path.exists(dataset_path):
    class_names = sorted([d for d in os.listdir(dataset_path) if os.path.isdir(os.path.join(dataset_path, d))])
    print(f"Detected classes: {class_names}")
else:
    class_names = ['Disease_Chilli', 'Disease_Tomato', 'Healthy_Chilli', 'Healthy_Tomato']

last_result = {"status": "idle", "condition": "unknown", "disease_name": "none"}

@app.route('/upload', methods=['POST'])
def upload():
    global last_result
    print("\n" + "="*40)
    print("[UPLOAD] Image received from ESP32-CAM")

    file_bytes = np.frombuffer(request.data, np.uint8)
    img = cv2.imdecode(file_bytes, cv2.IMREAD_COLOR)

    if img is None:
        print("[ERROR] Could not decode image!")
        return jsonify({"error": "No image received"}), 400

    print(f"[UPLOAD] Image size: {img.shape[1]}x{img.shape[0]} px")

    # Save incoming image for data collection
    if not os.path.exists("captured_images"):
        os.makedirs("captured_images")
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    save_path = f"captured_images/img_{timestamp}.jpg"
    cv2.imwrite(save_path, img)
    print(f"[UPLOAD] Image saved to: {save_path}")

    if model is None:
        # Mock mode
        predicted_class = random.choice(class_names)
        print(f"[MOCK MODE] Randomly predicted: {predicted_class}")
    else:
        # Preprocessing
        img_resized = cv2.resize(img, (224, 224))
        img_array = tf.expand_dims(img_resized, 0)

        # Predict
        predictions = model.predict(img_array, verbose=0)
        score = tf.nn.softmax(predictions[0])
        confidence = 100 * np.max(score)
        
        # 🛡️ CONFIDENCE THRESHOLD (60%)
        # If the AI is less than 60% sure, it's probably not a plant it knows.
        threshold = 60.0 
        if confidence < threshold:
            predicted_class = "Unknown"
            print(f"[AI] Low confidence ({confidence:.2f}%). Classifying as Unknown.")
        else:
            predicted_class = class_names[np.argmax(score)]
            print(f"[AI] Predicted class : {predicted_class}")
            print(f"[AI] Confidence      : {confidence:.2f}%")

    # Logic to trigger mist maker
    if "Disease" in predicted_class:
        condition = "disease"
    elif predicted_class == "Unknown":
        condition = "unknown"
    else:
        condition = "healthy"

    print(f"[RESULT] Condition set to: '{condition}'")
    print(f"[RESULT] Disease name    : '{predicted_class}'")

    last_result = {
        "status": "complete",
        "condition": condition,
        "disease_name": predicted_class
    }

    print("[UPLOAD] Result stored. Waiting for ESP32 to fetch it...")
    return "OK"

@app.route('/get_result', methods=['GET'])
def get_result():
    global last_result
    res = last_result.copy()

    print(f"\n[GET_RESULT] ESP32 is fetching result...")
    print(f"[GET_RESULT] Sending: status='{res['status']}', condition='{res['condition']}', disease='{res['disease_name']}'")

    if res["status"] == "complete":
        if res["condition"] == "disease":
            print("[GET_RESULT] >>> DISEASE detected! ESP32 should now trigger relay and buzzer.")
        else:
            print("[GET_RESULT] >>> Plant is HEALTHY. No relay action expected.")
        last_result = {"status": "idle", "condition": "unknown", "disease_name": "none"}
    else:
        print("[GET_RESULT] Status is 'idle' — no new result yet.")

    return jsonify(res)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=False)

import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, InputLayer, Dropout, Conv1D, Conv2D, Flatten, Reshape, MaxPooling1D, MaxPooling2D, AveragePooling2D, BatchNormalization, Permute, ReLU, Softmax
from tensorflow.keras.optimizers.legacy import Adam

EPOCHS = args.epochs or 50
LEARNING_RATE = args.learning_rate or 0.001
# If True, non-deterministic functions (e.g. shuffling batches) are not used.
# This is False by default.
ENSURE_DETERMINISM = args.ensure_determinism
# this controls the batch size, or you can manipulate the tf.data.Dataset objects yourself
BATCH_SIZE = args.batch_size or 32

# Add normalization layer for preprocessing
# Standard scaling (zero mean, unit variance)
normalizer = tf.keras.layers.Normalization(axis=-1)
normalizer.adapt(train_dataset.map(lambda x, y: x))

if not ENSURE_DETERMINISM:
    train_dataset = train_dataset.shuffle(buffer_size=BATCH_SIZE*4)
train_dataset = train_dataset.batch(BATCH_SIZE, drop_remainder=False)
validation_dataset = validation_dataset.batch(BATCH_SIZE, drop_remainder=False)

# model architecture
model = Sequential()
# Input layer (explicit)
model.add(InputLayer(input_shape=(3,)))  # 3 features: rmse_x, rmse_y, rmse_z
model.add(normalizer)

# Hidden layers with recommended architecture
model.add(Dense(16, activation='relu',
    activity_regularizer=tf.keras.regularizers.l2(0.0001)))
model.add(Dropout(0.2))

model.add(Dense(8, activation='relu',
    activity_regularizer=tf.keras.regularizers.l2(0.0001)))
model.add(Dropout(0.2))

# Output layer - using sigmoid for binary classification
model.add(Dense(classes, name='y_pred', activation='sigmoid' if classes == 1 else 'softmax'))

# this controls the learning rate
opt = Adam(learning_rate=LEARNING_RATE, beta_1=0.9, beta_2=0.999)

callbacks.append(BatchLoggerCallback(BATCH_SIZE, train_sample_count, epochs=EPOCHS, ensure_determinism=ENSURE_DETERMINISM))

# Early stopping to prevent overfitting
early_stopping = tf.keras.callbacks.EarlyStopping(
    monitor='val_loss',
    patience=10,
    restore_best_weights=True
)
callbacks.append(early_stopping)

# train the neural network
# Use binary_crossentropy for binary classification
loss_function = 'binary_crossentropy' if classes == 1 else 'categorical_crossentropy'
model.compile(loss=loss_function, optimizer=opt, metrics=['accuracy'])
model.fit(train_dataset, epochs=EPOCHS, validation_data=validation_dataset, 
          verbose=2, callbacks=callbacks, class_weight=ei_tensorflow.training.get_class_weights(Y_train))

# Use this flag to disable per-channel quantization for a model.
# This can reduce RAM usage for convolutional models, but may have
# an impact on accuracy.
disable_per_channel_quantization = False
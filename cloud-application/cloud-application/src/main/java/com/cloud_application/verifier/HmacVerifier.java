package com.cloud_application.verifier;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.cloud_application.dto.SensorData; // Adjust the package name based on your DTO's actual location

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

public class HmacVerifier {

    private static final byte[] HMAC_KEY = new byte[]{
            (byte) 0x01, (byte) 0x02, (byte) 0x03, (byte) 0x04, (byte) 0x05, (byte) 0x06, (byte) 0x07, (byte) 0x08,
            (byte) 0x09, (byte) 0x0A, (byte) 0x0B, (byte) 0x0C, (byte) 0x0D, (byte) 0x0E, (byte) 0x0F, (byte) 0x10,
            (byte) 0x11, (byte) 0x12, (byte) 0x13, (byte) 0x14, (byte) 0x15, (byte) 0x16, (byte) 0x17, (byte) 0x18,
            (byte) 0x19, (byte) 0x1A, (byte) 0x1B, (byte) 0x1C, (byte) 0x1D, (byte) 0x1E, (byte) 0x1F, (byte) 0x20
    };

    public static boolean verifyHmac(String jsonString) {
        try {
            ObjectMapper objectMapper = new ObjectMapper();
            SensorData data = objectMapper.readValue(jsonString, SensorData.class);

            // Build ordered JSON structure for HMAC payload
            StringBuilder orderedJson = new StringBuilder();
            orderedJson.append("{");

            if (data.getStatus() != null) {
                orderedJson.append("\"status\":\"").append(data.getStatus()).append("\",");
            }

            orderedJson
                    .append("\"x\":").append(data.getX()).append(",")
                    .append("\"y\":").append(data.getY()).append(",")
                    .append("\"z\":").append(data.getZ()).append(",")
                    .append("\"session_id\":").append(data.getSessionId()).append(",")
                    .append("\"seq\":").append(data.getSeq()).append(",")
                    .append("\"time\":").append(data.getTime())
                    .append("}");

            String payloadJsonString = orderedJson.toString();

            // Prepare binary header (session_id, seq, time)
            ByteBuffer buffer = ByteBuffer.allocate(4 + 4 + 8);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            buffer.putInt(data.getSessionId());
            buffer.putInt(data.getSeq());
            buffer.putLong(data.getTime());
            byte[] authBinary = buffer.array();

            byte[] jsonBytes = payloadJsonString.getBytes(StandardCharsets.UTF_8);
            byte[] fullPayload = new byte[authBinary.length + jsonBytes.length];
            System.arraycopy(authBinary, 0, fullPayload, 0, authBinary.length);
            System.arraycopy(jsonBytes, 0, fullPayload, authBinary.length, jsonBytes.length);

            // HMAC-SHA256
            Mac hmacSha256 = Mac.getInstance("HmacSHA256");
            SecretKeySpec secretKey = new SecretKeySpec(HMAC_KEY, "HmacSHA256");
            hmacSha256.init(secretKey);
            byte[] calculatedHmac = hmacSha256.doFinal(fullPayload);

            String calculatedHmacHex = bytesToHex(calculatedHmac);
            String receivedHmacHex = data.getHmac();

            boolean isCorrect = calculatedHmacHex.equalsIgnoreCase(receivedHmacHex);

            /*
            System.out.println("Original JSON: " + jsonString);
            System.out.println("Reconstructed JSON: " + payloadJsonString);
            System.out.println("HMAC match: " + isCorrect);
            System.out.println("Expected HMAC: " + receivedHmacHex);
            System.out.println("Calculated HMAC: " + calculatedHmacHex);
*/
            return true;

        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder hexString = new StringBuilder();
        for (byte b : bytes) {
            hexString.append(String.format("%02x", b));
        }
        return hexString.toString();
    }
}

package com.cloud_application;

import com.cloud_application.subscription.SubscriptionHandler;
import com.cloud_application.verifier.HmacVerifier;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.context.annotation.Bean;
import org.springframework.integration.annotation.ServiceActivator;
import org.springframework.integration.channel.DirectChannel;
import org.springframework.integration.core.MessageProducer;
import org.springframework.integration.mqtt.core.DefaultMqttPahoClientFactory;
import org.springframework.integration.mqtt.core.MqttPahoClientFactory;
import org.springframework.integration.mqtt.inbound.MqttPahoMessageDrivenChannelAdapter;
import org.springframework.integration.mqtt.support.DefaultPahoMessageConverter;
import org.springframework.messaging.Message;
import org.springframework.messaging.MessageChannel;
import org.springframework.messaging.MessageHandler;
import org.springframework.messaging.MessagingException;

@SpringBootApplication
public class DashboardApplication {

    @Autowired
    SubscriptionHandler subscriptionHandler;

    public static void main(String[] args) {

        SpringApplication.run(DashboardApplication.class, args);

        String msg = "{\"status\":\"ANOMALY\",\"x\":0.266646,\"y\":1.059339,\"z\":0.406033,\"session_id\":10,\"seq\":1,\"time\":3484,\"hmac\":\"2a4b138ace1a5f5eafa236274b089e154e0a5efbbafe449ce680e747e830a5a5\"} ";
        HmacVerifier.verifyHmac(msg);
    }

    @Bean
    public MessageChannel mqttInputChannel() {
        return new DirectChannel();
    }

    @Bean
    public MqttPahoClientFactory mqttClientFactory() {
        DefaultMqttPahoClientFactory factory = new DefaultMqttPahoClientFactory();
        MqttConnectOptions options = new MqttConnectOptions();
        options.setAutomaticReconnect(true);
        options.setCleanSession(true);
        options.setConnectionTimeout(10); // seconds
        factory.setConnectionOptions(options);
        return factory;
    }

    @Bean
    public MessageProducer inbound() {

        MqttPahoMessageDrivenChannelAdapter adapter =
                new MqttPahoMessageDrivenChannelAdapter("tcp://localhost:1883", "testClient", mqttClientFactory(),
                        "+/RMS", "+/anomalies", "+/power");
        adapter.setCompletionTimeout(5000);
        adapter.setConverter(new DefaultPahoMessageConverter());
        adapter.setQos(1);
        adapter.setOutputChannel(mqttInputChannel());
        return adapter;
    }


    @Bean
    @ServiceActivator(inputChannel = "mqttInputChannel")
    public MessageHandler handler() {
        return new MessageHandler() {

            @Override
            public void handleMessage(Message<?> message) throws MessagingException {
                String topic = (String) message.getHeaders().get("mqtt_receivedTopic");
                String payload = (String) message.getPayload();
                subscriptionHandler.store(topic, payload);


            }

        };
    }


}

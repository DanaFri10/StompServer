package bgu.spl.net.srv;

import java.io.IOException;
import java.util.List;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId) throws IOException;

    void subscribe(String channel, int connectionId, int subscriptionId);

    void unsubscribe(int subscriptionId, int connectionId);

    int connectionCount();
    List<Integer> getSubscribed(String channel);
    int getSubscriptionId(String channel, int connectionId);
}

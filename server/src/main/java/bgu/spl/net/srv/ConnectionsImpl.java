package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

public class ConnectionsImpl<T> implements Connections<T>{
    private Map<Integer, ConnectionHandler<T>> connections;
    private Map<String, List<Integer>> subscriptions;
    private Map<Integer, Map<Integer, String>> clientSubscriptionIds;

    private int lastMessageId;

    public ConnectionsImpl(){
        lastMessageId = 0;
        connections = new HashMap<>();
        subscriptions = new HashMap<>();
        clientSubscriptionIds = new HashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> cH = connections.get(connectionId);
        try {
            cH.send(msg);
        }
        catch (Exception e){
            return false;
        }
        return true;
    }

    @Override
    public void send(String channel, T msg) {
        List<Integer> connectionIds = subscriptions.get(channel);
        for(int connectedId : connectionIds){
            send(connectedId, msg);
        }
    }

    @Override
    public void disconnect(int connectionId){
        connections.remove(connectionId);
        clientSubscriptionIds.remove(connectionId);
        for(String channel : subscriptions.keySet()){
            List<Integer> connectionIds = subscriptions.get(channel);
            if(connectionIds.contains(connectionId)){
                connectionIds.remove(Integer.valueOf(connectionId));
            }
        }
    }

    public void addConnection(int connectionId, ConnectionHandler<T> cH){
        connections.put(connectionId, cH);
    }

    public void subscribe(String channel, int connectionId, int subscriptionId){
        if(subscriptions.get(channel) == null){
            subscriptions.put(channel, new LinkedList<Integer>());
        }
        subscriptions.get(channel).add(connectionId);
        if(clientSubscriptionIds.get(connectionId) == null){
            clientSubscriptionIds.put(connectionId, new HashMap<>());
        }
        clientSubscriptionIds.get(connectionId).put(subscriptionId, channel);
    }

    public void unsubscribe(int subscriptionId, int connectionId){
        String channel = clientSubscriptionIds.get(connectionId).get(subscriptionId);
        clientSubscriptionIds.get(connectionId).remove(subscriptionId);
        subscriptions.get(channel).remove(Integer.valueOf(connectionId));
    }

    public int connectionCount()
    {
        return connections.size();
    }

    @Override
    public List<Integer> getSubscribed(String channel) {
        if(!subscriptions.containsKey(channel)){
            return new LinkedList<>();
        }
        return subscriptions.get(channel);
    }

    @Override
    public int getSubscriptionId(String channel, int connectionId) {
        if(!clientSubscriptionIds.containsKey(connectionId))
            return -1;

        for(int id : clientSubscriptionIds.get(connectionId).keySet()){
            if(clientSubscriptionIds.get(connectionId).get(id).equals(channel)){
                return id;
            }
        }
        return -1;
    }
    
    public boolean isClientSubscribed(int connectionId, int subId)
    {
        return clientSubscriptionIds.get(connectionId).containsKey(subId);
    }

    public int getMessageId(){
        return lastMessageId++;
    }
}

package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Frame;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

public class Users {
    private Map<String, User> users;

    public Users()
    {
        users=new HashMap<>();
    }

    public Frame login(String login, String password, int connectionId, String acceptVersion)
    {
        ConcurrentMap<String, String> headers=new ConcurrentHashMap<String, String>();
        headers.put("accept-version", acceptVersion);

        if(users.containsKey(login)) //user is registered
        {
            User user=users.get(login);
            if(user.getIsLoggedIn())
            {
                return new Frame("ERROR", null, "User already logged in.");
            }
            if(!user.getPassword().equals(password))
                return new Frame("ERROR",null,"Wrong password."); //wrong password
            
            user.setLoggedIn(true);
        }
        
        //user is not registered
        User newUser = new User(login, password, connectionId);
        users.put(login, newUser);
        return new Frame("CONNECTED",headers,"Login successful.");//connected frame
    }

    public void logout(String login)
    {
        users.get(login).setLoggedIn(false);

    }

    
}

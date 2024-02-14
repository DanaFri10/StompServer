package bgu.spl.net.impl.stomp;

public class User {
    private String login;
    private String password;
    private boolean isLoggedIn;
    private int connectionId;

    public User(String login, String password, int conconnectionId)
    {
        this.login=login;
        this.password=password;
        this.isLoggedIn=true;
        this.connectionId=conconnectionId;
    }


    public String getLogin()
    {
        return login;
    }

    public String getPassword()
    {
        return password;
    }

    public boolean getIsLoggedIn()
    {
        return isLoggedIn;
    }

    public int getConnectionId()
    {
        return connectionId;
    }

    public boolean setLoggedIn(boolean login)
    {
        if(!login)
            connectionId=-1;
        return isLoggedIn=login;
    }

}

using UnityEditor;

public class ServiceAuth
{
    private const string strServiceAuth = "TEST_SERVICE_AUTH";
    private string strServiceAuthKey;
    public ServiceAuth()
    {
        /**********************************
         * serviceAuth에 대한 처리
         **********************************/
        string osVersion = _getOS();
        strServiceAuthKey = strServiceAuth + "_" + osVersion;
    }

    private string _getOS()
    {
        return "WINDOW";
    }

    public string _getServiceAuthKey()
    {
        return strServiceAuthKey;
    }
}
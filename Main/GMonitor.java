import java.io.*;
import java.net.*;
import java.net.URLEncoder;
import java.net.URLDecoder;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.Map;

public class GMonitor {
    public static final String defaultServerIP = "127.0.0.1";
    public static final int defaultServerPort = 9000;
    //private Socket socket = null;

    private String serverIP;
    private int serverPort;
    //private Socket socket = null;

    public GMonitor() {
        this.serverIP = GMonitor.defaultServerIP;
        this.serverPort = GMonitor.defaultServerPort;
    }

    public GMonitor(int _port) {
        this.serverIP = GMonitor.defaultServerIP;
        this.serverPort = _port;
    }

    public GMonitor(String _ip, int _port) {
        this.serverIP = _ip;
        this.serverPort = _port;
    }

    public String sendGet(String param) {
        String url = "http://" + this.serverIP + ":" + this.serverPort;
        String result = "";
        BufferedReader in = null;
        System.out.println("parameter: " + param);

        try {
            param = URLEncoder.encode(param, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            String urlNameString = url + "/" + param;
            System.out.println("request: " + urlNameString);
            URL realUrl = new URL(urlNameString);
            // 打开和URL之间的连接
            URLConnection connection = realUrl.openConnection();
            // 设置通用的请求属性
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent",
                                          "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");
            // 建立实际的连接
            connection.connect();
            // 获取所有响应头字段
            Map<String, List<String>> map = connection.getHeaderFields();
            // 遍历所有的响应头字段
            for (String key : map.keySet()) {
                System.out.println(key + "--->" + map.get(key));
            }
            // 定义 BufferedReader输入流来读取URL的响应
            in = new BufferedReader(new InputStreamReader(
                                        connection.getInputStream()));
            String line;
            while ((line = in.readLine()) != null) {
                result += line;
            }
        } catch (Exception e) {
            System.out.println("error in get request: " + e);
            e.printStackTrace();
        }
        // 使用finally块来关闭输入流
        finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (Exception e2) {
                e2.printStackTrace();
            }
        }
        return result;
    }

    public boolean connect() {
        return true;
    }

    public boolean disconnect() {
        return true;
    }

    public static void main(String[] args) {
        if (args.length != 3) {
            System.err.println("Invalid parameter, the parammeters should be operation, ip/url and port!");
            return;
        }
        String op = args[0];
        String serverIP = args[1];
        int serverPort = Integer.parseInt(args[2]);

        // initialize the GStore server's IP address and port.
        GMonitor gm = new GMonitor(serverIP, serverPort);

        gm.connect();

        String cmd = "?operation=" + op;
        String msg = gm.sendGet(cmd);

        gm.disconnect();
        System.out.println(msg);
    }
}

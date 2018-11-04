/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package offline2;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

/**
 *
 * @author Heisenberg
 */
public class finalServer {

    private static int workerThreadNo, id;

    public static void main(String[] args) throws IOException {
        workerThreadNo = 0;
        id = 0;
        int port = 6789;
        ServerSocket ss = new ServerSocket(port);
        System.out.println("Server Socket at port " + port + " is opened successfully.");

        while (true) {
            Socket s = ss.accept();
            finalHTTPWorker worker = new finalHTTPWorker(s, id);
            Thread t = new Thread(worker);
            t.start();
            workerThreadNo++;
            id++;
        }
    }
}

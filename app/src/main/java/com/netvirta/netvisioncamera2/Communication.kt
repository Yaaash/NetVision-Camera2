package com.netvirta.netvisioncamera2

import android.app.Activity
import android.content.Context
import android.net.ConnectivityManager
import android.util.Log
import java.io.BufferedWriter
import java.io.IOException
import java.io.OutputStreamWriter
import java.io.PrintWriter
import java.net.InetAddress
import java.net.Socket
import java.net.UnknownHostException

class Communication {

    var mSocket: Socket? = null

    internal var serverThread: Thread? = null
    val SERVERPORT = 6000
    private val SERVER_IP = "192.168.2.77"

    private var mSocketOpened = 0
    fun send_message(str: String): Int {
        if (mSocketOpened == 0)
            return 0

        try {
            val out = PrintWriter(
                BufferedWriter(
                    OutputStreamWriter(mSocket?.getOutputStream())
                ),
                true
            )
            out.println(str)
            Log.d(javaClass.simpleName, "bob send OK $str")

        } catch (e: UnknownHostException) {
            e.printStackTrace()
        } catch (e: IOException) {
            e.printStackTrace()
        } catch (e: Exception) {
            e.printStackTrace()
        }

        return 0
    }

    fun send_lane(str: String): Int {
        var str = str
        str = "<cmd>laneloc=$str</cmd>"
        return send_message(str)
    }


    internal inner class ServerThread : Runnable {
        override fun run() {
            mSocketOpened = 0
            try {
                Log.d(TAG, "bob comm run ...")
                val serverAddr = InetAddress.getByName(SERVER_IP)
                mSocket = Socket(serverAddr, SERVERPORT)
                mSocketOpened = 1
                Log.d(TAG, "bob open socket")
            } catch (e1: UnknownHostException) {
                e1.printStackTrace()
            } catch (e1: IOException) {
                e1.printStackTrace()
            }

            if (mSocketOpened == 0) {
                Log.d(TAG, " bob open socket ERROR")
            }
        }
    }

    fun start(): Int {
        Log.d(TAG, "bob start...")
        serverThread = Thread(ServerThread())
        serverThread!!.start()
        return 0
    }

    fun stop(): Int {
        if (serverThread != null) {
            try {
                serverThread!!.join()
                serverThread = null

            } catch (e: InterruptedException) {
                e.printStackTrace()
            }

        }
        return 0
    }
}
package org.xapian.util;

import java.net.URL;

public class LoaderUtils {

    public static boolean runningInJar() {
        String classResourcePath = LoaderUtils.class.getName()
	    .replace('.', '/') + ".class";
        URL resource = Thread.currentThread()
                .getContextClassLoader()
                .getResource(classResourcePath);
        if (resource != null) {
            return resource.getProtocol().contains("jar");
        } else {
            throw new RuntimeException("Could not find LoaderUtils.class");
        }
    }

    public static void loadDynamic() {
        if(runningInJar()) {
            // Try to find the library as a resource

	    URL libraryUrl = Thread.currentThread().getContextClassLoader()
		.getResource("libxapian_jni.so");
            if (libraryUrl != null) {
		String libraryPath = libraryUrl.getPath();
                System.load(libraryPath);
            } else {
                throw new RuntimeException("Could not find 'libxapian_jni.so");
            }
        } else {
            // Library path has been explicitly set so lets try to use it
            try {
                System.loadLibrary("xapian_jni");
            } catch (UnsatisfiedLinkError error) {
		System.err.println("derp");
                String libraryPath = Thread.currentThread()
		    .getContextClassLoader().getResource("libxapian_jni.so")
		    .getPath();
                System.load(libraryPath);
            }
        }
    }
}

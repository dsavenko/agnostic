package com.dsavenko.agnostic;

import java.io.File;
import java.io.IOException;

public class App {

	private static final String DEFAULT_CONFIG_FILE = "agnostic.yaml";

	public static void main(String[] args) throws IOException {
		try {
			if (1 > args.length) {
				help();
			} else {
				for (String cmd : args) {
					runCmd(cmd);
				}
			}
		} catch (Throwable e) {
			e.printStackTrace();
		}
		System.exit(0);
	}

	private static void runCmd(String cmd) {
		switch (cmd) {
			case "checkout":
				checkout();
				break;
			case "build-up":
				buildUp();
				break;
			case "build":
				build();
				break;
			default:
				help();
				throw new AgnosticException("Unknown command: " + cmd);
		}
	}

	private static File currentDir() {
		return new File(System.getProperty("user.dir"));
	}

	private static String currentProject() {
		return currentDir().getAbsoluteFile().getName();
	}

	private static File config() {
		File currentDir = currentDir();
		File parent = currentDir.getParentFile();
		File parentConfig = new File(parent, DEFAULT_CONFIG_FILE);
		if (!parentConfig.exists()) {
			throw new AgnosticException("Config file not found: " + parentConfig.getAbsolutePath());
		}
		return parentConfig;
	}

	private static Components components(File config) {
		return Components.load(config);
	}

	private static void checkout() {
		components(new File(DEFAULT_CONFIG_FILE)).checkoutAll();
	}

	private static void buildUp() {
		components(config()).buildUp(currentProject());
	}

	private static void build() {
		components(config()).build(currentProject());
	}

	private static void help() {
		System.out.println("java -jar agnostic.jar <command> [command] [command] ...\n" +
			"Commands: help, checkout, build, build-up");
	}

}

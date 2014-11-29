package com.dsavenko.agnostic;

public class AgnosticException extends RuntimeException {

	public AgnosticException() {
	}

	public AgnosticException(String message) {
		super(message);
	}

	public AgnosticException(String message, Throwable cause) {
		super(message, cause);
	}

	public AgnosticException(Throwable cause) {
		super(cause);
	}

	public AgnosticException(String message, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(message, cause, enableSuppression, writableStackTrace);
	}
}

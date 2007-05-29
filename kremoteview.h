/***************************************************************************
                   kremoteview.h  -  widget that shows the remote framebuffer
                             -------------------
    begin                : Wed Dec 25 23:58:12 CET 2002
    copyright            : (C) 2002-2003 by Tim Jansen
    email                : tim@tjansen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KREMOTEVIEW_H
#define KREMOTEVIEW_H

#include <QWidget>

/**
 * Generic widget that displays a remote framebuffer.
 * Implement this if you want to add another backend.
 *
 * Things to take care of:
 * @li The KRemoteView is responsible for its size. In
 *     non-scaling mode, set the fixed size of the widget
 *     to the remote resolution. In scaling mode, set the
 *     maximum size to the remote size and minimum size to the
 *     smallest resolution that your scaler can handle.
 * @li if you override mouseMoveEvent()
 *     you must ignore the QEvent, because the KRDC widget will
 *     need it for stuff like toolbar auto-hide and bump
 *     scrolling. If you use x11Event(), make sure that
 *     MotionNotify events will be forwarded.
 *
 */
class KRemoteView : public QWidget
{
	Q_OBJECT
public:

    Q_ENUMS(Quality)

    enum Quality {
        Unknown,
        High,
        Medium,
        Low
    };

    /**
    * Describes the state of a local cursor, if there is such a concept in the backend.
    * With local cursors, there are two cursors: the cursor on the local machine (client),
    * and the cursor on the remote machine (server). Because there is usually some lag,
    * some backends show both cursors simultanously. In the VNC backend the local cursor
    * is a dot and the remote cursor is the 'real' cursor, usually an arrow.
    */

    Q_ENUMS(DotCursorState)

    enum DotCursorState {
        CursorOn,  ///< Always show local cursor (and the remote one).
        CursorOff, ///< Never show local cursor, only the remote one.
        /// Try to measure the lag and enable the local cursor if the latency is too high.
        CursorAuto
    };

    /**
    * State of the connection. The state of the connection is returned
    * by @ref KRemoteView::status().
    *
    * Not every state transition is allowed. You are only allowed to transition
    * a state to the following state, with three exceptions:
    * @li You can move from every state directly to Disconnected
    * @li You can move from every state except Disconnected to
    *     Disconnecting
    * @li You can move from Disconnected to Connecting
    *
    * @ref KRemoteView::setStatus() will follow this rules for you.
    * (If you add/remove a state here, you must adapt it)
    */

    Q_ENUMS(RemoteStatus)

    enum RemoteStatus {
        Connecting     = 0,
        Authenticating = 1,
        Preparing      = 2,
        Connected      = 3,
        Disconnecting  = -1,
        Disconnected   = -2
    };

    Q_ENUMS(ErrorCode)

    enum ErrorCode {
        None = 0,
        Internal,
        Connection,
        Protocol,
        IO,
        Name,
        NoServer,
        ServerBlocked,
        Authentication
    };

	KRemoteView(QWidget *parent = 0);

	virtual ~KRemoteView();

	/**
	 * Checks whether the backend supports scaling. The
	 * default implementation returns false.
	 * @return true if scaling is supported
	 * @see scaling()
	 */
	virtual bool supportsScaling() const;

	/**
	 * Checks whether the widget is in scale mode. The
	 * default implementation always returns false.
	 * @return true if scaling is activated. Must always be
	 *         false if @ref supportsScaling() returns false
	 * @see supportsScaling()
	 */
	virtual bool scaling() const;

	/**
	 * Checks whether the backend supports the concept of local cursors. The
	 * default implementation returns false.
	 * @return true if local cursors are supported/known
	 * @see DotCursorState
	 * @see showDotCursor()
	 * @see dotCursorState()
	 */
	virtual bool supportsLocalCursor() const;

	/**
	 * Sets the state of the dot cursor, if supported by the backend.
	 * The default implementation does nothing.
	 * @param state the new state (CursorOn, CursorOff or
	 *        CursorAuto)
	 * @see dotCursorState()
	 * @see supportsLocalCursor()
	 */
	virtual void showDotCursor(DotCursorState state);

	/**
	 * Returns the state of the local cursor. The default implementation returns
	 * always CursorOff.
	 * @return true if local cursors are supported/known
	 * @see showDotCursor()
	 * @see supportsLocalCursor()
	 */
	virtual DotCursorState dotCursorState() const;

	/**
	 * Checks whether the view is in view-only mode. This means
	 * that all input is ignored.
	 */
	virtual bool viewOnly() = 0;

	/**
	 * Returns the resolution of the remote framebuffer.
	 * It should return a null @ref QSize when the size
	 * is not known.
	 * The backend must also emit a @ref changeSize()
	 * when the size of the framebuffer becomes available
	 * for the first time or the size changed.
	 * @return the remote framebuffer size, a null QSize
	 *         if unknown
	 */
	virtual QSize framebufferSize() = 0;

	/**
	 * Initiate the disconnection. This doesn't need to happen
	 * immediately. The call must not block.
	 * @see isQuitting()
	 */
	virtual void startQuitting() = 0;

	/**
	 * Checks whether the view is currently quitting.
	 * @return true if it is quitting
	 * @see startQuitting()
	 * @see setStatus()
	 */
	virtual bool isQuitting() = 0;

	/**
	 * Returns the host the view is connected to.
	 * @return the host the view is connected to
	 */
	virtual QString host() = 0;

	/**
	 * Returns the port the view is connected to.
	 * @return the port the view is connected to
	 */
	virtual int port() = 0;

	/**
	 * Initialize the view (for example by showing configuration
	 * dialogs to the user) and start connecting. Should not block
	 * without running the event loop (so displaying a dialog is ok).
	 * When the view starts connecting the application must call
	 * @ref setStatus() with the status Connecting.
	 * @return true if successful (so far), false
	 *         otherwise
	 * @see connected()
	 * @see disconnected()
	 * @see disconnectedError()
	 * @see statusChanged()
	 */
	virtual bool start() = 0;

	/**
	 * Returns the current status of the connection.
	 * @return the status of the connection
	 * @see setStatus()
	 */
	RemoteStatus status();

public slots:
        /**
	 * Called to enable or disable scaling.
	 * Ignored if @ref supportsScaling() is false.
	 * The default implementation does nothing.
	 * @param s true to enable, false to disable.
	 * @see supportsScaling()
	 * @see scaling()
	 */
        virtual void enableScaling(bool s);

	/**
	 * Enables/disables the view-only mode.
	 * Ignored if @ref supportsScaling() is false.
	 * The default implementation does nothing.
	 * @param s true to enable, false to disable.
	 * @see supportsScaling()
	 * @see viewOnly()
	 */
	virtual void setViewOnly(bool s) = 0;

	/**
	 * Called to let the backend know it when
	 * we switch from/to fullscreen.
	 * @param on true when switching to fullscreen,
	 *           false when switching from fullscreen.
	 */
	virtual void switchFullscreen(bool on);

        /**
	 * Sends a key to the remote server.
	 * @param k the key to send
	 */
        virtual void pressKey(XEvent *k) = 0;

signals:
        /**
	 * Emitted when the size of the remote screen changes. Also
	 * called when the size is known for the first time.
	 * @param x the width of the screen
	 * @param y the height of the screen
	 */
	void changeSize(int w, int h);

	/**
	 * Emitted when the view connected successfully.
	 */
	void connected();

	/**
	 * Emitted when the view disconnected without error.
	 */
	void disconnected();

	/**
	 * Emitted when the view disconnected with error.
	 */
	void disconnectedError();

	/**
	 * Emitted when the status of the view changed.
	 * @param s the new status
	 */
	void statusChanged(KRemoteView::RemoteStatus s);

	/**
	 * Emitted when the password dialog is shown or hidden.
	 * @param b true when the dialog is shown, false when it has
	 *               been hidden
	 */
	void showingPasswordDialog(bool b);

	/**
	 * Emitted when the mouse on the remote side has been moved.
	 * @param x the new x coordinate
	 * @param y the new y coordinate
	 * @param buttonMask the mask of mouse buttons (bit 0 for first mouse
	 *                   button, 1 for second button etc)a
	 */
	void mouseStateChanged(int x, int y, int buttonMask);

protected:
	/**
	 * The status of the remote view.
	 */
	RemoteStatus m_status;

	/**
	 * Set the status of the connection.
	 * Emits a statusChanged() signal.
	 * Note that the states need to be set in a certain order,
	 * see @ref Status. setStatus() will try to do this
	 * transition automatically, so if you are in Connecting
	 * and call setStatus(Preparing), setStatus() will
	 * emit a Authenticating and then Preparing.
	 * If you transition backwards, it will emit a
	 * Disconnected before doing the transition.
	 * @param s the new status
	 */
	virtual void setStatus(RemoteStatus s);
};

#endif

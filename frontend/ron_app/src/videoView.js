import React, { useState, useEffect } from 'react';
import Button from 'react-bootstrap/Button';
import { ReactComponent as handle_svg } from "./assets/joy.svg"


const FloatingPictureInPicture = (props) => {
    const [position, setPosition] = useState({ top: 100, left: 100 });
    const [size, setSize] = useState({ width: 350, height: 350 });
    const [isDragging, setIsDragging] = useState(false);
    const [isResizing, setIsResizing] = useState(false);
    const [delta, setDelta] = useState({ x: 0, y: 0 });
    const [isFullScreen, setIsFullScreen] = useState(false);

    const handleMouseDown = (event) => {
        if (event.target.tagName === 'path') {
            setIsResizing(true);
        } else {
            setIsDragging(true);
        }
        setDelta({
            x: event.clientX - position.left,
            y: event.clientY - position.top,
        });
    };

    const handleMouseMove = (event) => {
        if (isDragging) {
            setPosition({
                top: event.clientY - delta.y,
                left: event.clientX - delta.x,
            });
        } else if (isResizing) {
            setSize({
                width: event.clientX - position.left,
                height: event.clientY - position.top,
            });
        }
    };

    const handleMouseUp = () => {
        setIsDragging(false);
        setIsResizing(false);
    };

    const handleTouchStart = (event) => {
        event.preventDefault();
        handleMouseDown(event.touches[0]);
    };

    const handleTouchMove = (event) => {
        event.preventDefault();
        handleMouseMove(event.touches[0]);
    };

    const handleTouchEnd = (event) => {
        event.preventDefault();
        handleMouseUp();
    };

    useEffect(() => {
        const preventDefault = (e) => e.preventDefault();
        if (isDragging || isResizing) {
            document.addEventListener('touchmove', preventDefault, { passive: false });
        } else {
            document.removeEventListener('touchmove', preventDefault);
        }
        return () => {
            document.removeEventListener('touchmove', preventDefault);
        };
    }, [isDragging, isResizing]);

    useEffect(() => {
        if (isFullScreen) {
            setPosition({ top: 0, left: 0 });
            setSize({ width: window.innerWidth, height: window.innerHeight });
        } else {
            setPosition({ top: 100, left: 100 });
            setSize({ width: 350, height: 350 });
        }
    }, [isFullScreen]); 

    return (
        <div
            style={{
                position: 'fixed',
                top: position.top,
                left: position.left,
                zIndex: 9999,
                width: [size.width, 'px'].join(''),
                cursor: isDragging ? 'grabbing' : 'grab',
                borderRadius: '15px',
                outline: "1px solid white",
            }}
            onMouseMove={handleMouseMove}
            onMouseUp={handleMouseUp}
            onTouchStart={handleTouchStart}
            onTouchMove={handleTouchMove}
            onTouchEnd={handleTouchEnd}
        >
            <div className="window" onMouseDown={handleMouseDown} style={{padding: '0px', margin: '0px'}}>
                <div style={{
                    position: 'absolute',
                    width: "40px",
                    right: "-20px",
                    bottom: "-20px",
                    padding: "0px 0px",
                    margin: "0px 0px",
                    }}
                    onMouseDown={handleMouseDown}
                    onMouseUp={handleMouseUp}
                    onTouchStart={handleTouchStart}
                    onTouchMove={handleTouchMove}
                    onTouchEnd={handleTouchEnd}
                >
                    <svg
                        viewBox='0 0 60 60'
                        xmlns="http://www.w3.org/2000/svg"
                    >
                        <path
                            style={{
                                fill: "rgba(216, 216, 216, 0)",
                                strokeWidth: "20px",
                                strokeLinecap: 'round',
                                stroke: 'rgb(235, 235, 235)',
                            }}
                            d="M 50 10 C 50 30 30 50 10 50"
                        />
                    </svg>
                </div>
                
                {props.content}
                <Button
                    variant="success"
                    style={{
                        position: 'absolute',
                        bottom: '35%',
                        left: '10%',
                    }}
                    onClick={() => {
                        props.setZoom(zoom => zoom + 10);
                    }}
                >
                    +
                </Button>
                <Button
                    variant="success"
                    style={{
                        position: 'absolute',
                        bottom: '35%',
                        left: '30%',
                    }}
                    onClick={() => {
                        props.setZoom(zoom => zoom - 10);
                    }}
                >
                    -
                </Button>

                <Button
                    variant="outline-dark"
                    style={{
                        position: 'absolute',
                        bottom: '10%',
                        left: '10%',
                    }}
                    onClick={() => {
                        setIsFullScreen(!isFullScreen);
                    }}
                >
                      FULl SCREEN
                </Button>
            </div>
        </div>
    );
};

export default FloatingPictureInPicture;
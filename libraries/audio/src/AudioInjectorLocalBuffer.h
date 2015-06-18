//
//  AudioInjectorLocalBuffer.h
//  libraries/audio/src
//
//  Created by Stephen Birarda on 2014-11-11.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AudioInjectorLocalBuffer_h
#define hifi_AudioInjectorLocalBuffer_h

#include <QtCore/qiodevice.h>

#include <glm/detail/func_common.hpp>

class AudioInjectorLocalBuffer : public QIODevice {
    Q_OBJECT
public:
    AudioInjectorLocalBuffer(const QByteArray& rawAudioArray, QObject* parent);
    
    void stop();
    
    bool seek(qint64 pos);
    
    qint64 readData(char* data, qint64 maxSize);
    qint64 writeData(const char* data, qint64 maxSize) { return 0; }
    
    void setShouldLoop(bool shouldLoop) { _shouldLoop = shouldLoop; }
    void setCurrentOffset(int currentOffset) { _currentOffset = currentOffset; }
    void setVolume(float volume) { _volume = glm::clamp(volume, 0.0f, 1.0f); }
    
private:
    qint64 recursiveReadFromFront(char* data, qint64 maxSize);
    
    QByteArray _rawAudioArray;
    bool _shouldLoop;
    bool _isStopped;
    
    int _currentOffset;
    float _volume;
};

#endif // hifi_AudioInjectorLocalBuffer_h
/*
SPDX-FileCopyrightText: 2012 Simon A. Eugster (Granjow)  <simon.eu@gmail.com>
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "audioStreamInfo.h"
#include "kdenlivesettings.h"

#include "kdenlive_debug.h"
#include <KLocalizedString>
#include <cstdlib>

AudioStreamInfo::AudioStreamInfo(const std::shared_ptr<Mlt::Producer> &producer, int audioStreamIndex, bool playlist)
    : m_audioStreamIndex(audioStreamIndex)
    , m_ffmpegAudioIndex(0)
    , m_samplingRate(48000)
    , m_channels(2)
    , m_bitRate(0)
{
    // Fetch audio streams
    int streams = producer->get_int("meta.media.nb_streams");
    if (streams == 0) {
        if (playlist) {
            // Playlist clips do not provide stream info
            m_audioStreams.insert(0, i18n("Audio"));
            // TODO: compute playlist channels
            m_audioChannels.insert(0, 2);
        } else {
            if (producer->get("mlt_service") == QLatin1String("blipflash")) {
                // used in tests
                m_audioStreams.insert(0, i18n("Audio"));
                m_audioChannels.insert(0, 2);
            }
        }
    }
    int streamIndex = 1;
    for (int ix = 0; ix < streams; ix++) {
        char property[200];
        snprintf(property, sizeof(property), "meta.media.%d.stream.type", ix);
        QString type = producer->get(property);
        if (type == QLatin1String("audio")) {
            memset(property, 0, 200);
            snprintf(property, sizeof(property), "meta.media.%d.codec.channels", ix);
            int chan = producer->get_int(property);
            memset(property, 0, 200);
            snprintf(property, sizeof(property), "kdenlive:streamname.%d", ix);
            QString channelDescription = producer->get(property);
            if (channelDescription.isEmpty()) {
                channelDescription = QStringLiteral("%1|").arg(streamIndex++);
                switch (chan) {
                case 1:
                    channelDescription.append(i18n("Mono "));
                    break;
                case 2:
                    channelDescription.append(i18n("Stereo "));
                    break;
                default:
                    channelDescription.append(i18n("%1 channels ", chan));
                    break;
                }
                // Frequency
                memset(property, 0, 200);
                snprintf(property, sizeof(property), "meta.media.%d.codec.sample_rate", ix);
                QString frequency(producer->get(property));
                if (frequency.endsWith(QLatin1String("000"))) {
                    frequency.chop(3);
                    frequency.append(i18n("kHz "));
                } else {
                    frequency.append(i18n("Hz "));
                }
                channelDescription.append(frequency);
                memset(property, 0, 200);
                snprintf(property, sizeof(property), "meta.media.%d.codec.name", ix);
                channelDescription.append(producer->get(property));
            } else {
                streamIndex++;
            }
            m_audioStreams.insert(ix, channelDescription);
            m_audioChannels.insert(ix, chan);
        }
    }
    QString active = producer->get("kdenlive:active_streams");
    updateActiveStreams(active);
    if (m_audioStreams.count() > 1 && active.isEmpty()) {
        // initialize enabled streams
        QStringList streamString;
        for (int streamIx : std::as_const(m_activeStreams)) {
            streamString << QString::number(streamIx);
        }
        producer->set("kdenlive:active_streams", streamString.join(QLatin1Char(';')).toUtf8().constData());
    }

    if (audioStreamIndex > -1) {
        QByteArray key;
        key = QStringLiteral("meta.media.%1.codec.sample_fmt").arg(audioStreamIndex).toLocal8Bit();
        m_samplingFormat = QString::fromLatin1(producer->get(key.data()));

        key = QStringLiteral("meta.media.%1.codec.sample_rate").arg(audioStreamIndex).toLocal8Bit();
        m_samplingRate = producer->get_int(key.data());

        key = QStringLiteral("meta.media.%1.codec.bit_rate").arg(audioStreamIndex).toLocal8Bit();
        m_bitRate = producer->get_int(key.data());

        key = QStringLiteral("meta.media.%1.codec.channels").arg(audioStreamIndex).toLocal8Bit();
        m_channels = producer->get_int(key.data());
        setAudioIndex(producer, m_audioStreamIndex);
    }
}

AudioStreamInfo::~AudioStreamInfo() = default;

int AudioStreamInfo::samplingRate() const
{
    return m_samplingRate;
}

int AudioStreamInfo::channels(int stream) const
{
    if (m_audioChannels.contains(stream)) {
        return m_audioChannels.value(stream);
    }
    return m_channels;
}

QMap<int, QString> AudioStreamInfo::streams() const
{
    return m_audioStreams;
}

QMap<int, int> AudioStreamInfo::streamChannels() const
{
    return m_audioChannels;
}

int AudioStreamInfo::channelsForStream(int stream) const
{
    if (m_audioChannels.contains(stream)) {
        return m_audioChannels.value(stream);
    }
    return m_channels;
}

QList<int> AudioStreamInfo::activeStreamChannels() const
{
    if (m_activeStreams.size() == 1 && m_activeStreams.contains(INT_MAX)) {
        return m_audioChannels.values();
    }
    QList<int> activeChannels;
    QMapIterator<int, QString> i(m_audioStreams);
    while (i.hasNext()) {
        i.next();
        if (m_activeStreams.contains(i.key())) {
            activeChannels << m_audioChannels.value(i.key());
        }
    }
    return activeChannels;
}

QMap<int, QString> AudioStreamInfo::activeStreams() const
{
    QMap<int, QString> active;
    QMapIterator<int, QString> i(m_audioStreams);
    if (m_activeStreams.size() == 1 && m_activeStreams.contains(INT_MAX)) {
        active.insert(INT_MAX, i18n("Merged streams"));
    } else {
        while (i.hasNext()) {
            i.next();
            if (m_activeStreams.contains(i.key())) {
                active.insert(i.key(), i.value());
            }
        }
    }
    return active;
}

int AudioStreamInfo::bitrate() const
{
    return m_bitRate;
}

int AudioStreamInfo::audio_index() const
{
    return m_audioStreamIndex;
}

int AudioStreamInfo::ffmpeg_audio_index() const
{
    return m_ffmpegAudioIndex;
}

void AudioStreamInfo::dumpInfo() const
{
    qCDebug(KDENLIVE_LOG) << "Info for audio stream " << m_audioStreamIndex << "\n\tChannels: " << m_channels << "\n\tSampling rate: " << m_samplingRate
                          << "\n\tBit rate: " << m_bitRate;
}

void AudioStreamInfo::setAudioIndex(const std::shared_ptr<Mlt::Producer> &producer, int ix)
{
    m_audioStreamIndex = ix;
    if (ix > -1) {
        int streams = producer->get_int("meta.media.nb_streams");
        QList<int> audioStreams;
        for (int i = 0; i < streams; ++i) {
            QByteArray propertyName = QStringLiteral("meta.media.%1.stream.type").arg(i).toLocal8Bit();
            QString type = producer->get(propertyName.data());
            if (type == QLatin1String("audio")) {
                audioStreams << i;
            }
        }
        if (audioStreams.count() > 1 && m_audioStreamIndex < audioStreams.count()) {
            m_ffmpegAudioIndex = audioStreams.indexOf(m_audioStreamIndex);
        }
    }
}

void AudioStreamInfo::updateActiveStreams(const QString &activeStreams)
{
    // -1 = disable all audio
    // empty = enable all audio or first depending on config
    m_activeStreams.clear();
    if (activeStreams.isEmpty()) {
        switch (KdenliveSettings::multistream()) {
        case 1:
            // Enable first stream only
            m_activeStreams << m_audioStreams.firstKey();
            break;
        case 2:
            // Enable the first two streams only
            {
                QList<int> str = m_audioStreams.keys();
                while (!str.isEmpty()) {
                    m_activeStreams << str.takeFirst();
                    if (m_activeStreams.size() == 2) {
                        break;
                    }
                }
                break;
            }
        default:
            // Enable all streams
            m_activeStreams = m_audioStreams.keys();
            break;
        }
        return;
    }
    QStringList st = activeStreams.split(QLatin1Char(';'));
    for (const QString &s : std::as_const(st)) {
        m_activeStreams << s.toInt();
    }
}

void AudioStreamInfo::renameStream(int ix, const QString &streamName)
{
    if (m_audioStreams.contains(ix)) {
        m_audioStreams.insert(ix, streamName);
    }
}

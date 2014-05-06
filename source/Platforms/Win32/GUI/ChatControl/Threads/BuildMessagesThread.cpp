#include <windows.h>

#include "BuildMessagesThread.h"
#include "../ChatControl.h"
#include "../ChatControlMessageFrame.h"
#include "../Messages/ChatControlMessage.h"
#include "../Messages/ChatControlMessageImage.h"
#include "../Messages/ChatControlMessageLocation.h"
#include "../Messages/ChatControlMessageText.h"
#include "../Messages/ChatControlMessageVideo.h"
#include "../../../../../VectorUtils.h"
#include "../../../../../Synchronization/Lock.h"
#include "../../../../../WhatsApp/Chat.h"
#include "../../../../../WhatsApp/Message.h"

BuildMessagesThread::BuildMessagesThread(HWND window, Lock &lock, WhatsappChat *chat, std::vector<ChatControlMessageFrame *> &messages, Smileys &smileys, Font &dateFont, ImageDecoder &imageDecoder)
	: window(window), lock(lock), chat(chat), messages(messages), smileys(smileys), dateFont(dateFont), imageDecoder(imageDecoder)
{
}

BuildMessagesThread::~BuildMessagesThread()
{
}

void BuildMessagesThread::run()
{
	if (!lock.tryLockWhile(running))
	{
		return;
	}

	clearVector(messages);

	if (chat != NULL)
	{
		std::vector<WhatsappMessage *> &messages = chat->getMessages(running);
		for (std::vector<WhatsappMessage *>::iterator it = messages.begin(); it != messages.end(); ++it)
		{
			if (!running)
			{
				break;
			}

			WhatsappMessage &message = **it;
			ChatControlMessageFrame *messageFrame = NULL;

			int color;

			if (message.isFromMe())
			{
				color = RGB(190, 240, 150);
			}
			else
			{
				color = RGB(230, 230, 240);
			}

			switch (message.getMediaWhatsappType())
			{
				case MEDIA_WHATSAPP_TEXT:
				{
					messageFrame = new ChatControlMessageFrame(new ChatControlMessageText(message, 0, smileys), 0, color, dateFont);
				} break;
				case MEDIA_WHATSAPP_IMAGE:
				{
					if (message.getRawDataSize() > 0 && message.getRawData() != NULL)
					{
						messageFrame = new ChatControlMessageFrame(new ChatControlMessageImage(message, 0, imageDecoder), 0, color, dateFont);
					}
				} break;
				case MEDIA_WHATSAPP_VIDEO:
				{
					if (message.getRawDataSize() > 0 && message.getRawData() != NULL)
					{
						messageFrame = new ChatControlMessageFrame(new ChatControlMessageVideo(message, 0, imageDecoder), 0, color, dateFont);
					}
				} break;
				case MEDIA_WHATSAPP_LOCATION:
				{
					messageFrame = new ChatControlMessageFrame(new ChatControlMessageLocation(message, 0, imageDecoder), 0, color, dateFont);
				} break;
			}

			if (messageFrame != NULL)
			{
				this->messages.push_back(messageFrame);
			}
		}
	}

	if (running)
	{
		PostMessage(window, WM_CHATCONTROL, CHAT_CONTROL_BUILDING_MESSAGES_FINISHED, 0);
	}

	lock.unlock();
}

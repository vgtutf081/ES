#pragma once 

namespace ES::Threading {

template<typename Element>
	class Queue {
	public:
		Queue(size_t count) {
			_handle = xQueueCreate(count, sizeof(Element));
		}

		~Queue() {
			vQueueDelete(_handle);
		}

		bool receive(Element &result, TickType_t wait = portMAX_DELAY) {
			return (xQueueReceive(_handle, &result, wait) != pdTRUE);

		}

        bool receiveFromISR(Element &result, TickType_t wait = portMAX_DELAY) {
			return (xQueueReceiveFromISR(_handle, &result, pdFALSE) != pdTRUE);

		}

		bool sendFromISR(const Element& element, TickType_t wait = portMAX_DELAY, bool yieldFromISR = true) {
			return (xQueueSendFromISR(_handle, &element, pdFALSE) != pdTRUE);

		}

        bool send(const Element& element, TickType_t wait = portMAX_DELAY, bool yieldFromISR = true) {
			return (xQueueSend(_handle, &element, wait) == pdTRUE);

		}

		size_t size() const {
			return uxQueueMessagesWaiting(_handle);
		}

	private:
		QueueHandle_t _handle;
	};
}
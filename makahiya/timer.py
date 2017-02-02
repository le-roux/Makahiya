import time
import asyncio
import logging
from .websockets import plants, send_to_socket
import datetime
from .constants import constants
log = logging.getLogger(__name__)

async def clock(plant_id, absolute=0, hour=0, minute=0, second=0, sound=0, light=0):
    cur = datetime.datetime.now()
    if absolute:
        date = datetime.datetime(cur.year, cur.month, cur.day, hour, minute, second)
        # If it's tomorrow, increase day by 1
        if (hour < cur.hour
            or (hour == cur.hour and minute < cur.minute)
            or (hour == cur.hour and minute == cur.minute and second < cur.second)):
            d = datetime.timedelta(days=1)
            date = date + d
        delta = date - cur
    else:
        delta = datetime.timedelta(hours=hour, minutes=minute, seconds=second)
    try:
        if light == 1:
            msg = 'alarm ' + str(int(delta.total_seconds())) + \
                constants.ALL_OFF + \
                ' ' + str(constants.LED1_R) + ' ' + str(255) + \
                ' ' + str(constants.LED1_G) + ' ' + str(0) + \
                ' ' + str(constants.LED1_B) + ' ' + str(0) + \
                constants.SET_LED_ON[1]
        elif light == 2:
            msg = 'alarm ' + str(int(delta.total_seconds())) + \
                constants.ALL_OFF + \
                ' ' + str(constants.LED1_R) + ' ' + str(0) + \
                ' ' + str(constants.LED1_G) + ' ' + str(255) + \
                ' ' + str(constants.LED1_B) + ' ' + str(0) + \
                constants.SET_LED_ON[1]

        await send_to_socket(plants, plant_id, msg)
        log.debug("Request sent")
    except KeyError as e:
        log.debug("Request error")
    return cur + delta

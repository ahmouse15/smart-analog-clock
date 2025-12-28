import AsyncStorage from "@react-native-async-storage/async-storage";
import { Alarm } from "../types/alarm";
import { createTimeObject } from "./time";
import { Temporal } from "@js-temporal/polyfill";

//Internally, Time object is stored and saved with just the hours and minutes.

export async function getAllAlarms(): Promise<Alarm[]> {
    const alarmsRaw = await AsyncStorage.getItem("alarms");

    const alarms: Alarm[] = JSON.parse(alarmsRaw || "", function (this: Alarm, key, val) {
        if (key === "time") {
            return Temporal.PlainTime.from(val);
        }

        return val;
    });

    return alarms || [];
}

export async function getAlarmById(id: string): Promise<Alarm | null> {
    const alarms = await getAllAlarms();
    if (alarms == null) return null;

    return alarms.find(x => x.id == id) || null;
}

export function saveAllAlarms(alarms: Alarm[]) {
    return AsyncStorage.setItem("alarms", JSON.stringify(alarms));
}

export async function saveSingleAlarm(alarm: Alarm) {
    let read = await getAllAlarms();
    read[read.findIndex(x => x.id == alarm.id)] = alarm;
    console.log(read);
    saveAllAlarms(read);
}
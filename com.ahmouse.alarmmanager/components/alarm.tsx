import * as React from 'react';
import { Button, Divider, Modal, Portal, Switch, Text, TouchableRipple, useTheme } from 'react-native-paper';
import { TimePickerModal } from 'react-native-paper-dates';
import { StyleSheet, View } from 'react-native';
import { FlatList } from 'react-native';
import { UUIDTypes, v4 as uuidv4 } from 'uuid';
import { useState } from 'react';
import { createTimeObject } from '../lib/time';
import { Alarm } from '../types/alarm';
import { saveAllAlarms, saveSingleAlarm } from '../lib/storage';
import { Temporal } from '@js-temporal/polyfill';

type AlarmProps = {
  alarm: Alarm
};

type AlarmListItemProps = AlarmProps;

type AlarmListProps = {
  allAlarms: Alarm[]
};

type AlarmSettingsModalProps = AlarmProps & {
  visible: boolean,
  onSave: (alarm: Alarm) => void,
  onClose: () => void
};

function toTimeStr(time: Temporal.PlainTime) {
  return time.toLocaleString(undefined, {timeStyle: "short"});
}

export function AlarmListItem(props: AlarmListItemProps) {
  const [modalVisible, setModalVisible] = React.useState(false);
  const [alarm, setAlarm] = React.useState(props.alarm);

  const theme = useTheme();

  const styles = StyleSheet.create({
    rippleContainer: {
      height: "auto",
      width: '100%',
    },
    container: {
      height: "auto",
      width: '100%',
      backgroundColor: 'none',
      alignItems: 'center',
      flexDirection: 'row',
      justifyContent: 'space-between',
      padding: 10,

    },
    leftContainer: {
    },
    rightContainer: {
    }
  });

  function updateAlarmDetails(newAlarm: Alarm) {
    let changes = {...newAlarm, enabled: alarm.enabled};
    setAlarm(changes);
    
    saveSingleAlarm(changes);
  }

  function enableAlarm(enabled: boolean) {
    let changes = {...alarm, enabled: enabled};
    setAlarm(changes);
    
    saveSingleAlarm(changes);
  }

  return (
    <TouchableRipple
      style={styles.rippleContainer}
      onPress={() => {setModalVisible(true)}}
      onLongPress={() => {}}
      borderless={true}
    >
      <View style={styles.container}>
        <View style={styles.leftContainer}>
          <Text variant="displayLarge">{toTimeStr(alarm.time)}</Text>
          <Text variant="titleMedium">Schedule</Text>
        </View>
        <View style={styles.rightContainer}>
          <Switch value={alarm.enabled} onValueChange={enableAlarm}/>
        </View>
        <AlarmSettingsModal 
          onClose={() => {setModalVisible(false)}}
          visible={modalVisible}
          alarm={alarm}
          onSave={updateAlarmDetails}
        />
      </View>
    </TouchableRipple>
  );
}

export function AlarmSettingsModal(props: AlarmSettingsModalProps) {
  const [pickerVisible, setPickerVisible] = React.useState(false);
  const [pendingAlarm, setPendingAlarm] = React.useState(props.alarm);
  
  const theme = useTheme();
  

  const styles = StyleSheet.create({
    modal: {
    },
    container: {
      justifyContent: 'space-between',
      height: 'auto',
      width: '80%',
      maxHeight: 400,
      maxWidth: 300,
      backgroundColor: theme.colors.surface,
      borderRadius: 20,
      alignSelf: 'center',
      padding: 10
    },
    buttonContainer: {
      flexDirection: 'row',
      justifyContent: 'flex-end',
      bottom: 0,
    },
    button: {
      margin: 2
    }
  });

  function updatePendingTime(hoursAndMinutes: {hours: number; minutes: number;}) {
    setPendingAlarm({...pendingAlarm, time: new Temporal.PlainTime(hoursAndMinutes.hours, hoursAndMinutes.minutes)});
    setPickerVisible(false);
  }

  function savePendingAlarm() {
    //Save changes
    props.onSave(pendingAlarm);
    props.onClose();
  };

  return (
      <Portal>
        <Modal style={styles.modal} visible={props.visible}>
          <View style={styles.container}>
            <Text variant="displayLarge" onPress={() => {setPickerVisible(true)}}>
              {toTimeStr(pendingAlarm.time)}
            </Text>

            <View style={styles.buttonContainer}>
              <Button style={styles.button} onPress={props.onClose}>Close</Button>
              <Button style={styles.button} onPress={savePendingAlarm}>Save</Button>
            </View>
          <TimePickerModal
            visible={pickerVisible}
            onDismiss={() => {setPickerVisible(false)}}
            onConfirm={updatePendingTime}
            hours={pendingAlarm.time.hour}
            minutes={pendingAlarm.time.minute}
          />
          </View>
        </Modal>
      </Portal>
  );
}

export function AlarmList(props: AlarmListProps) {
  const theme = useTheme();

  const styles = StyleSheet.create({
    container: {
      flex: 1,
      backgroundColor: 'transparent',
      alignItems: 'center',
      justifyContent: 'flex-start',
      marginTop: 20
    }
  });

  return (
      <View style={styles.container}>
        <FlatList 
          data={props.allAlarms}
          renderItem={({item}) => <AlarmListItem alarm={item} />}
          // @ts-ignore (upstream has incorrect type defs for this prop)
          ItemSeparatorComponent={<Divider horizontalInset={true}/>}
        />
      </View>
  );
}
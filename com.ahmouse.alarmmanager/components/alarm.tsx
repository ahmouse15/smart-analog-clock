import * as React from 'react';
import { Button, Modal, Portal, Switch, Text, TouchableRipple, useTheme } from 'react-native-paper';
import { TimePickerModal } from 'react-native-paper-dates';
import { StyleSheet, View } from 'react-native';
import { FlatList } from 'react-native';
import { UUIDTypes, v4 as uuidv4 } from 'uuid';
import { useState } from 'react';

type Alarm = {
  id: string,
  time: Date
};

type AlarmProps = {
  alarm: Alarm
};

type AlarmListProps = {
  items: Alarm[]
};

type AlarmSettingsModalProps = AlarmProps & {
  visible: boolean,
  closeModal: () => void
};

function toTimeStr(time: Date) {
  let hours = time.getHours().toString();
  let minutes = time.getMinutes().toString();

  return [hours.padStart(2, '0'), minutes.padEnd(2, '0')].join(':');
}

export function AlarmListItem(props: AlarmProps) {
  const theme = useTheme();

  const [modalVisible, setModalVisible] = useState(false);

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
      pointerEvents: 'box-none'
    },
    leftContainer: {
    },
    rightContainer: {
    }
  });

  // Extract hours and minutes in locale-specific manner
  let timeStr = toTimeStr(props.alarm.time);

  return (
    <TouchableRipple style={styles.rippleContainer} disabled={false} onPress={() => {setModalVisible(true)}}>
      <View style={styles.container}>
        <View style={styles.leftContainer}>
          <Text variant="displayLarge">{timeStr}</Text>
          <Text variant="titleMedium">Schedule</Text>
        </View>
        <View style={styles.rightContainer}>
          <Switch/>
        </View>
        <AlarmSettingsModal 
          closeModal={() => {setModalVisible(false)}}
          visible={modalVisible}
          alarm={props.alarm}
        />
      </View>
    </TouchableRipple>
  );
}

export function AlarmSettingsModal(props: AlarmSettingsModalProps) {
  const theme = useTheme();

  const [pickerVisible, setPickerVisible] = useState(false);

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

  //Pending changes stored here
  let alarmPending: Alarm = {...props.alarm};

  function updatePendingTime(hoursAndMinutes: {hours: number; minutes: number;}) {
    alarmPending.time.setHours(hoursAndMinutes.hours, hoursAndMinutes.minutes);

    setPickerVisible(false);
  }

  function saveAlarm() {
    //TODO: Commit alarmPending changes
    
    props.closeModal();
  };

  return (
      <Portal>
        <Modal style={styles.modal} visible={props.visible}>
          <View style={styles.container}>
            <Text variant="displayLarge" onPress={() => {setPickerVisible(true)}}>
              {toTimeStr(props.alarm.time)}
            </Text>

            <View style={styles.buttonContainer}>
              <Button style={styles.button} onPress={props.closeModal}>Close</Button>
              <Button style={styles.button} onPress={saveAlarm}>Save</Button>
            </View>
          <TimePickerModal
            visible={pickerVisible}
            onDismiss={() => {setPickerVisible(false)}}
            onConfirm={updatePendingTime}
            hours={props.alarm.time.getHours()}
            minutes={props.alarm.time.getMinutes()}
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
          data={props.items}
          renderItem={({item}) => <AlarmListItem alarm={item} />}
        />
      </View>
  );
}
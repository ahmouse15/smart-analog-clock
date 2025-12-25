import * as React from 'react';
import { Button, Modal, Portal, Switch, Text, TouchableRipple, useTheme } from 'react-native-paper';
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
  let timeStr = [props.alarm.time.getHours(), props.alarm.time.getMinutes()].join(':');

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

  const styles = StyleSheet.create({
    container: {
      flex: 1,
      backgroundColor: theme.colors.primaryContainer,
      alignItems: 'center',
      justifyContent: 'flex-start',
      marginTop: 20
    }
  });

  function saveAlarm() {
    //TODO: Alarm save logic here
    
    props.closeModal();
  };

  return (
      <Portal>
        <Modal visible={props.visible}>
          <Text>Placeholder</Text>
          <Button onPress={props.closeModal}>Close</Button>
          <Button onPress={saveAlarm}>Save</Button>
        </Modal>
      </Portal>
  );
}

export function AlarmList(props: AlarmListProps) {
  const theme = useTheme();

  const styles = StyleSheet.create({
    container: {
      flex: 1,
      backgroundColor: theme.colors.primaryContainer,
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
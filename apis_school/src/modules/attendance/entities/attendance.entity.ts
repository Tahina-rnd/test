import {
  Entity,
  Column,
  PrimaryGeneratedColumn,
  CreateDateColumn,
  UpdateDateColumn,
  DeleteDateColumn,
  ManyToOne,
  JoinColumn,
  Index,
  Unique,
} from 'typeorm';
import { Student } from '../../students/entities/student.entity';

export enum AttendanceStatus {
  PRESENT = 'present',
  ABSENT = 'absent',
  LATE = 'late',
  EXCUSED = 'excused',
}

export enum SessionType {
  LECTURE = 'lecture',
  LAB = 'lab',
  TUTORIAL = 'tutorial',
  SEMINAR = 'seminar',
  EXAM = 'exam',
}

@Entity('attendance')
@Unique(['studentId', 'sessionDate', 'subjectCode', 'sessionType'])
@Index(['studentId', 'sessionDate'])
@Index(['subjectCode', 'sessionDate'])
export class Attendance {
  @PrimaryGeneratedColumn('uuid')
  id: string;

  @CreateDateColumn()
  createdAt: Date;

  @UpdateDateColumn()
  updatedAt: Date;

  @DeleteDateColumn()
  deletedAt?: Date;

  @Column({ name: 'student_id' })
  studentId: string;

  @ManyToOne(() => Student)
  @JoinColumn({ name: 'student_id' })
  student: Student;

  @Column({ name: 'subject_code', length: 20 })
  subjectCode: string;

  @Column({ name: 'subject_name', length: 100 })
  subjectName: string;

  @Column({ name: 'session_date' })
  sessionDate: Date;

  @Column({
    type: 'enum',
    enum: SessionType,
    default: SessionType.LECTURE,
  })
  sessionType: SessionType;

  @Column({
    type: 'enum',
    enum: AttendanceStatus,
    default: AttendanceStatus.PRESENT,
  })
  status: AttendanceStatus;

  @Column({ name: 'check_in_time', nullable: true })
  checkInTime?: Date;

  @Column({ name: 'check_out_time', nullable: true })
  checkOutTime?: Date;

  @Column({ name: 'duration_minutes', type: 'int', nullable: true })
  durationMinutes?: number;

  @Column({ name: 'notes', type: 'text', nullable: true })
  notes?: string;

  @Column({ name: 'marked_by', length: 100, nullable: true })
  markedBy?: string;

  @Column({ name: 'academic_year', length: 10 })
  academicYear: string;

  @Column({ name: 'semester', length: 20 })
  semester: string;
}

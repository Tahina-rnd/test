import { AttendanceStatus, SessionType } from '../entities/attendance.entity';

export class AttendanceResponseDto {
  id: string;
  studentId: string;
  subjectCode: string;
  subjectName: string;
  sessionDate: Date;
  sessionType: SessionType;
  status: AttendanceStatus;
  checkInTime?: Date;
  checkOutTime?: Date;
  durationMinutes?: number;
  notes?: string;
  markedBy?: string;
  academicYear: string;
  semester: string;
  createdAt: Date;
  updatedAt: Date;
}

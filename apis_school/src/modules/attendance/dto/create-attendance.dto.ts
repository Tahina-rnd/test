import {
  IsString,
  IsEnum,
  IsOptional,
  IsDateString,
  IsNumber,
  Min,
} from 'class-validator';
import { AttendanceStatus, SessionType } from '../entities/attendance.entity';

export class CreateAttendanceDto {
  @IsString()
  studentId: string;

  @IsString()
  subjectCode: string;

  @IsString()
  subjectName: string;

  @IsDateString()
  sessionDate: string;

  @IsEnum(SessionType)
  @IsOptional()
  sessionType?: SessionType;

  @IsEnum(AttendanceStatus)
  @IsOptional()
  status?: AttendanceStatus;

  @IsDateString()
  @IsOptional()
  checkInTime?: string;

  @IsDateString()
  @IsOptional()
  checkOutTime?: string;

  @IsNumber()
  @Min(0)
  @IsOptional()
  durationMinutes?: number;

  @IsString()
  @IsOptional()
  notes?: string;

  @IsString()
  @IsOptional()
  markedBy?: string;

  @IsString()
  academicYear: string;

  @IsString()
  semester: string;
}
